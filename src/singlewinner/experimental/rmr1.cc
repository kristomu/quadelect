// A lot of this has been stolen from the resistant set file.

#include "rmr1.h"

// beats[k][x][y] is true iff candidate x disqualifies y on
// every set of cardinality k and less.

typedef std::vector<std::vector<std::vector<bool> > > beats_tensor;

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

std::pair<ordering, bool> rmr1::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	beats_tensor beats = get_k_disqualifications(
			papers, hopefuls, num_candidates);

	// Start with everybody having max penalty (i.e. never winning).
	std::vector<int> first_victory_level(num_candidates,
		num_candidates + 1);

	// Make the case for each candidate.
	for (size_t cand = 0; cand < (size_t)num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
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
			// dto erase every defeat that can be erased, even though
			// we might already know we're defeated by someone else.

			for (size_t challenger = 0; challenger < (size_t)num_candidates;
				++challenger) {

				if (!hopefuls[challenger] || challenger == cand) {
					continue;
				}

				// Is the disqualification against us no longer active?
				// Break it.
				if (!beats[k][challenger][cand]) {
					current_beats[challenger][cand] = false;
				}

				defeated |= current_beats[challenger][cand];
			}

			if (!defeated) {
				first_victory_level[cand] = k;
			}
		}
	}

	// Turn the first level at which each candidate wins into an
	// ordering.
	ordering rmr_ordering;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		rmr_ordering.insert(candscore(cand, -first_victory_level[cand]));
	}

	return std::pair<ordering, bool>(rmr_ordering, false);

}
