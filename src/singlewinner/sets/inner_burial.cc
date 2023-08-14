#include "inner_burial.h"

std::pair<ordering, bool> inner_burial_set::elect_inner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	std::vector<std::vector<bool > > hopeful_power_set =
		power_set(hopefuls);

	// Create a vector of first preference scores for every possible
	// way to eliminate hopeful candidates. The accompanying num_candidates
	// gives the cardinality of the set, i.e. the number of remaining
	// hopefuls, so we can calculate the 1/n threshold. We also have a sum
	// of first preferences to odeal with equal rank/exhausted ballots.

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
	}

	std::vector<bool> included_candidates = hopefuls;

	condmat matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	// Now, for each pair of hopefuls, check if the first beats the second
	// pairwise and if all restricted elections has the first one at >1/n.

	for (int first_cand = 0; first_cand < num_candidates; ++first_cand) {
		if (!hopefuls[first_cand]) {
			continue;
		}
		for (int sec_cand = 0; sec_cand < num_candidates; ++sec_cand) {
			// Skip candidates who aren't hopeful or who have already
			// been disqualified.
			if (first_cand == sec_cand || !included_candidates[sec_cand]) {
				continue;
			}

			// Does A beat B pairwise? If not, no need to test.
			if (!matrix.beats(first_cand, sec_cand)) {
				continue;
			}

			bool passes_threshold = true;

			for (size_t subelection = 0; subelection < first_pref_scores.size()
				&& passes_threshold; ++subelection) {

				// If not both candidates are present in this subelection,
				// skip.
				if (!hopeful_power_set[subelection][first_cand]
					|| !hopeful_power_set[subelection][sec_cand]) {
					continue;
				}

				if (first_pref_scores[subelection][first_cand] <=
					num_remaining_voters[subelection]/
					num_remaining_candidates[subelection]) {
					passes_threshold = false;
				}
			}

			// If we still pass the threshold, the second candidate
			// is evicted.

			if (passes_threshold) {
				included_candidates[sec_cand] = false;
			}
		}
	}

	// Turn the set vector into an ordering and return.
	ordering inner_set;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		if (included_candidates[cand]) {
			inner_set.insert(candscore(cand, 1));
		} else {
			inner_set.insert(candscore(cand, 0));
		}
	}

	return std::pair<ordering, bool>(inner_set, false);

}