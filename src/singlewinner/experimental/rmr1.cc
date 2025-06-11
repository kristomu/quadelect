// A lot of this has been stolen from the resistant set file.

#include "rmr1.h"

#include "../pairwise/method.h"
#include "../sets/max_elements/schwartz.h"

#include <iostream>

disqual_tensor rmr1::get_k_disqualifications(const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates) const {

	subelections se;
	se.count_subelections(papers, hopefuls, false);

	return se.get_level_disqualifications(hopefuls, true);
}

condmat rmr1::get_defeating_matrix(const disqual_tensor & beats,
	const std::vector<bool> & hopefuls, int level) const {

	int num_candidates = hopefuls.size();

	// Define a Condorcet matrix for the weak defeats relation.
	// A weakly defeats B if the highest level k where either A~(k)~>B
	// or B~(k)~>A, we have A~(k)~>B.

	condmat matrix(num_candidates, 1, CM_PAIRWISE_OPP);

	for (int candidate = 0; candidate < num_candidates; ++candidate) {
		if (!hopefuls[candidate]) {
			continue;
		}

		for (int challenger = 0; challenger < candidate; ++challenger) {
			if (!hopefuls[challenger]) {
				continue;
			}

			if (beats[level][candidate][challenger]) {
				matrix.add(candidate, challenger, 1);
				matrix.add(challenger, candidate, 0);
			}

			if (beats[level][challenger][candidate]) {
				matrix.add(candidate, challenger, 0);
				matrix.add(challenger, candidate, 1);
			}
		}
	}

	return matrix;
}

ordering rmr1::iterative_schwartz(const disqual_tensor & beats,
	const std::vector<bool> & hopefuls) const {

	ordering base;
	bool first = true;
	size_t num_candidates = hopefuls.size();

	// Either direction works. But only this one appears to be Smith.
	//for (int level = num_candidates; level >= 2; --level) {
	for (int level = 2; level <= (int)num_candidates; ++level) {
		condmat matrix = get_defeating_matrix(beats, hopefuls, level);
		ordering this_level_out = schwartz_set().pair_elect(
				matrix, hopefuls, NULL, false).first;

		if (first) {
			base = this_level_out;
			first = false;
		} else {
			// Doing it the other way (i.e. current,base instead of
			// base,current) also is Resistant, for some reason.
			base = ordering_tools().ranked_tiebreak(base, this_level_out,
					num_candidates);
		}

	}

	return base;
}


int rmr1::get_score_defeated(const disqual_tensor & beats,
	const std::vector<bool> & hopefuls,
	int candidate, int num_candidates) const {

	// Start with everybody having max penalty (i.e. never winning).
	int first_victory_level = num_candidates + 1;
	if (!hopefuls[candidate]) {
		return first_victory_level;
	}

	// Start with the pairwise defeats.
	std::vector<std::vector<bool> > current_beats = beats[2];

	// And assume we're defeated.
	bool defeated = true;

	for (int k = 2; k <= num_candidates && defeated; ++k) {
		// Check if there are any defeats against us that we
		// can erase, and check if that makes us undefeated.

		defeated = false;

		// NOTE: We can *not* short-circuit here because we need
		// to erase every defeat that can be erased, even though
		// we might already know we're defeated by someone else.

		for (int challenger = 0; challenger < num_candidates;
			++challenger) {

			if (!hopefuls[challenger] || challenger == candidate) {
				continue;
			}

			// Is the disqualification against us no longer active?
			// Break it.
			if (!beats[k][challenger][candidate]) {
				current_beats[challenger][candidate] = false;
			}

			defeated |= current_beats[challenger][candidate];
		}

		if (!defeated) {
			first_victory_level = k;
		}
	}

	return -first_victory_level;
}

int rmr1::get_score_defeating(const disqual_tensor & beats,
	const std::vector<bool> & hopefuls,
	int candidate, int num_candidates) const {

	if (!hopefuls[candidate]) {
		return 0;
	}

	bool defeating = true;
	int score = 0;

	for (int k = 2; k <= num_candidates && defeating; ++k) {
		// Check if we're defeating anyone else and how many
		// candidates are defeating us.

		defeating = false;
		int defeated_by = 0;

		for (int challenger = 0; challenger < num_candidates;
			++challenger) {

			if (!hopefuls[challenger] || challenger == candidate) {
				continue;
			}

			defeating |= beats[k][candidate][challenger];

			// Nonmonotone HACK, come up with something better later.
			if (beats[k][challenger][candidate]) {
				++defeated_by;
			}
		}

		if (defeating) {
			// HACK: ditto. Since defeated_by is always less than
			// num_candidates, this in effect works as "k, with the number
			// of candidates beating us breaking any ties".
			score = (k * num_candidates) - defeated_by;
		}
	}

	return score;
}

int rmr1::get_score_two_way(const disqual_tensor & beats,
	const std::vector<bool> & hopefuls,
	int candidate, int num_candidates) const {

	int score = -(num_candidates+1);

	if (!hopefuls[candidate]) {
		return score;
	}

	// Could possibly break ties to try to deter nonmonotonicity, like
	// this: of every other candidate defeating someone on this level
	// but not yet being undefeated, how close are they to being undefeated
	// on subelections containing the candidate in question? Closer is
	// worse.

	// Or some kind of DSV where f(A, B) checks who wins when A,
	// alternatively B, is made to be top-ranked. But I'll have to take
	// a break before doing anything that complex.

	for (int k = 2; k <= num_candidates; ++k) {
		bool defeated = false;
		bool defeating_someone = false;

		for (int challenger = 0; challenger < num_candidates;
			++challenger) {

			if (candidate == challenger) {
				continue;
			}

			defeated |= beats[k][challenger][candidate];
			defeating_someone |= beats[k][candidate][challenger];
		}

		if (!defeated && defeating_someone) {
			return -k;
		}
	}

	return score;
}

std::pair<ordering, bool> rmr1::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	disqual_tensor beats = get_k_disqualifications(
			papers, hopefuls, num_candidates);

	if (chosen_type == RMR_SCHWARTZ_EXP) {
		return std::pair<ordering, bool>(
				iterative_schwartz(beats, hopefuls), false);
	}

	ordering rmr_ordering;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		int score;

		assert(chosen_type != RMR_SCHWARTZ_EXP);

		switch (chosen_type) {
			case RMR_DEFEATED:
				score = get_score_defeated(beats,
						hopefuls, cand, num_candidates);
				break;
			case RMR_DEFEATING:
				score = get_score_defeating(beats,
						hopefuls, cand, num_candidates);
				break;
			case RMR_TWO_WAY:
				score = get_score_two_way(beats,
						hopefuls, cand, num_candidates);
				break;
			default:
				throw std::invalid_argument("RMRA1: Invalid type!");
		}

		rmr_ordering.insert(candscore(cand, score));
	}

	return std::pair<ordering, bool>(rmr_ordering, false);

}