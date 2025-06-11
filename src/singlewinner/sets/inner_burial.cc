#include "inner_burial.h"

std::pair<ordering, bool> inner_burial_set::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	subelections se;
	se.count_subelections(papers, hopefuls, false);

	condmat matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	std::vector<bool> undisqualified_candidates = hopefuls;

	// Now, for each pair of hopefuls, check if the first beats the second
	// pairwise and if all restricted elections has the first one at >1/n.

	for (int first_cand = 0; first_cand < num_candidates; ++first_cand) {
		if (!hopefuls[first_cand]) {
			continue;
		}
		for (int sec_cand = 0; sec_cand < num_candidates; ++sec_cand) {
			// Skip candidates who aren't hopeful or who have already
			// been disqualified.
			if (first_cand == sec_cand || !undisqualified_candidates[sec_cand]) {
				continue;
			}

			// Does A beat B pairwise? If not, no need to test.
			if (!matrix.beats(first_cand, sec_cand)) {
				continue;
			}

			bool passes_threshold = true;

			for (size_t subelection = 0; subelection < se.first_pref_scores.size()
				&& passes_threshold; ++subelection) {

				// If not both candidates are present in this subelection,
				// skip.
				if (!se.hopeful_power_set[subelection][first_cand]
					|| !se.hopeful_power_set[subelection][sec_cand]) {
					continue;
				}

				if (se.first_pref_scores[subelection][first_cand] <=
					se.num_remaining_voters[subelection]/
					se.num_remaining_candidates[subelection]) {
					passes_threshold = false;
				}
			}

			// If we still pass the threshold, we disqualify the
			// second candidate; mark him as such.

			if (passes_threshold) {
				undisqualified_candidates[sec_cand] = false;
			}
		}
	}

	// Turn the set vector into an ordering and return.
	ordering inner_set;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		if (undisqualified_candidates[cand]) {
			inner_set.insert(candscore(cand, 1));
		} else {
			inner_set.insert(candscore(cand, 0));
		}
	}

	return std::pair<ordering, bool>(inner_set, false);
}