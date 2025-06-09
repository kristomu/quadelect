#include "subelections.h"

void subelections::count_subelections(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	bool tiebreak) {

	included_candidates = hopefuls;
	num_candidates = hopefuls.size();
	hopeful_power_set =	power_set(hopefuls);

	// Create a vector of first preference scores for every possible
	// way to eliminate hopeful candidates. The accompanying num_candidates
	// gives the cardinality of the set, i.e. the number of remaining
	// hopefuls, so we can calculate the 1/n threshold. We also have a sum
	// of first preferences to deal with equal rank/exhausted ballots.

	first_pref_scores.clear();
	num_remaining_candidates.clear();
	num_remaining_voters.clear();

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

			double score = cand_and_score.get_score();

			if (tiebreak) {
				score += 1e-6 * score * cand_and_score.get_candidate_num() *
					num_candidates;
			}

			first_prefs_this_selection[cand_and_score.get_candidate_num()] =
				score;
			num_remaining_voters_here += score;
		}

		first_pref_scores.push_back(first_prefs_this_selection);
		num_remaining_candidates.push_back(num_remaining_candidates_here);
		num_remaining_voters.push_back(num_remaining_voters_here);
	}
}
