// A lot of this has been stolen from the resistant set file.

#include "rmr1.h"

#include "../pairwise/method.h"
#include "../sets/max_elements/schwartz.h"

#include <iostream>

beats_tensor rmr1::get_k_disqualifications(const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates) const {

	std::vector<std::vector<std::vector<bool> > > beats(
		num_candidates+1, std::vector<std::vector<bool> >(
			num_candidates, std::vector<bool>(num_candidates,
				true)));

	std::vector<std::vector<bool > > hopeful_power_set =
		power_set(hopefuls);

	// Create a vector of first preference scores for every possible
	// way to eliminate hopeful candidates. The accompanying num_candidates
	// gives the cardinality of the set, i.e. the number of remaining
	// hopefuls, so we can calculate the 1/n threshold. We also have a sum
	// of first preferences to odeal with equal rank/exhausted ballots.

	// This will also update the beats vector to be accurate *per level*,
	// i.e. [k][x][y] is true if x disqualifies y on every set of cardinality
	// *exactly* k. We adjust it to be cumulative later.

	std::vector<std::vector<double> > first_pref_scores;
	std::vector<int> num_remaining_candidates;
	std::vector<double> num_remaining_voters;

	for (const std::vector<bool> & candidate_selection: hopeful_power_set) {

		int num_remaining_candidates_here = 0;

		for (size_t i = 0; i < candidate_selection.size(); ++i) {
			if (candidate_selection[i]) {
				++num_remaining_candidates_here;
			}
		}

		ordering plur_outcome;

		if (num_remaining_candidates_here > 0) {
			plur_outcome = plurality_method.elect(papers,
					candidate_selection, num_candidates, NULL, false);
		}

		std::vector<double> first_prefs_this_selection(
			num_candidates, 0);
		double num_remaining_voters_here = 0;

		for (const candscore & cand_and_score: plur_outcome) {
			assert(cand_and_score.get_candidate_num() < (size_t)num_candidates);
			first_prefs_this_selection[cand_and_score.get_candidate_num()] =
				cand_and_score.get_score();
			num_remaining_voters_here += cand_and_score.get_score();
		}

		first_pref_scores.push_back(first_prefs_this_selection);
		num_remaining_candidates.push_back(num_remaining_candidates_here);
		num_remaining_voters.push_back(num_remaining_voters_here);

		// Update the beats matrix for this cardinality.

		for (int cand = 0; cand < num_candidates; ++cand) {
			// If not in the current set, ignore.
			if (!candidate_selection[cand]) {
				continue;
			}

			// If we're not disqualifying anyone, then we need to set
			// the disqualification relation to every other candidate
			// to false. In other words, if we're below or at the 1/k threshold,
			// then we can't be disqualifying anyone, so set every
			// disqualification by us of this cardinality to false.

			if (num_remaining_candidates_here * first_prefs_this_selection[cand] >
				num_remaining_voters_here) {
				continue;
			}

			for (int other_cand = 0; other_cand < num_candidates; ++other_cand) {
				if (!candidate_selection[other_cand]) {
					continue;
				}
				if (other_cand == cand) {
					continue;
				}

				beats[num_remaining_candidates_here][cand][other_cand] = false;
			}
		}
	}

	// Update the beats vector to be cumulative: for beats[k][x][y] to be true,
	// it has to currently be true, *and* every lower cardinality beats[...][x][y]
	// also has to be true, since if A ~(k)~> B, then A disqualifies B on every
	// subelection with both of less than or equal to cardinality k.

	// Another way of saying that is that if beats[k][x][y] is false for level k,
	// then it's false for every higher level. Thus we can just set every higher
	// level to false.

	for (size_t k = 0; k < beats.size(); ++k) {
		for (size_t cand = 0; cand < beats[k].size(); ++cand) {
			for (size_t other = 0; other < beats[k][cand].size(); ++other) {
				if (cand == other) {
					continue;
				}
				if (!beats[k][cand][other]) {
					// Propagate falses up the stack.
					for (size_t j = k; j < beats.size(); ++j) {
						beats[j][cand][other] = false;
					}
				}
			}
		}
	}

	return beats;
}

condmat rmr1::get_defeating_matrix(const beats_tensor & beats,
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

ordering rmr1::iterative_schwartz(const beats_tensor & beats,
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


int rmr1::get_score_defeated(const beats_tensor & beats,
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

int rmr1::get_score_defeating(const beats_tensor & beats,
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

int rmr1::get_score_two_way(const beats_tensor & beats,
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

	beats_tensor beats = get_k_disqualifications(
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
