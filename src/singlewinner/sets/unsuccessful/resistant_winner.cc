#include "resistant_winner.h"

// TODO: Call this a resistant winner *set* instead.
// and explain that it's a weak set, e.g. if A and B are
// tied and beat everybody else, then we output {A, B}.
// Or test strict stuff first, breaking neutrality.

// Most of this is copied from the inner burial/resistant set
// code. Maybe I should extract some stuff from it so I don't
// have to copy large chunks of code all the time. (See also rmr1.cc.)

std::pair<ordering, bool> resistant_winner_set::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	std::vector<std::vector<bool > > hopeful_power_set =
		power_set(hopefuls);

	// Start off with every candidate being a potential resistant
	// winner.

	std::vector<bool> winners = hopefuls;
	int cand;

	for (const std::vector<bool> & candidate_selection: hopeful_power_set) {

		// Get the number of non-eliminated candidates.

		int num_remaining_candidates_here = 0;

		for (cand = 0; cand < num_candidates; ++cand) {
			if (candidate_selection[cand]) {
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

		// For each candidate, check if he clears the 1/n threshold.
		// If not, then that candidate can't be a winner.

		for (cand = 0; cand < num_candidates; ++cand) {
			if (!candidate_selection[cand]) {
				continue;
			}

			if (first_prefs_this_selection[cand] <
				num_remaining_voters_here/num_remaining_candidates_here) {
				winners[cand] = false;
			}
		}
	}

	// Turn the set vector into an ordering and return.
	ordering winner_set;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		if (winners[cand]) {
			winner_set.insert(candscore(cand, 1));
		} else {
			winner_set.insert(candscore(cand, 0));
		}
	}

	return std::pair<ordering, bool>(winner_set, false);
}