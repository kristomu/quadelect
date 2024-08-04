#include "beat_chain.h"
#include "pairwise/matrix.h"

std::pair<ordering, bool> beat_chain::elect_inner(const
	election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	// We need to check if a candidate beats everybody ranked lower.
	// And to do that, we need a Condorcet matrix. Create one here.

	condmat condorcet_matrix(CM_WV);
	condorcet_matrix.count_ballots(papers, num_candidates);

	ordering base_ordering = base->elect(papers, hopefuls,
			num_candidates, cache, false);

	// Now every candidate's score is either the number of candidates
	// beaten by him, if no candidate ranked below him by the base
	// method beats him, or -1 otherwise. Although slower to calculate,
	// this handles ties without a bunch of special casing. (I can
	// always optimize later if needed.)

	ordering output_ordering;

	for (auto pos = base_ordering.rbegin(); pos != base_ordering.rend();
		++pos) {

		if (!hopefuls[pos->get_candidate_num()]) {
			continue;
		}

		bool beats_everybody_lower = true;
		int candidates_ranked_below = 0;

		// Check everybody ranked below pos. Our continuing criteria are:
		// - the candidate we're checking must be ranked below pos,
		// - and we must be beating everybody considered so far.
		for (auto ranked_below = base_ordering.rbegin();
			ranked_below->get_score() < pos->get_score()
			&& beats_everybody_lower; ++ranked_below) {

			if (!hopefuls[ranked_below->get_candidate_num()]) {
				continue;
			}

			if (condorcet_matrix.beats(pos->get_candidate_num(),
					ranked_below->get_candidate_num())) {
				++candidates_ranked_below;
			} else {
				beats_everybody_lower = false;
			}
		}

		if (beats_everybody_lower) {
			output_ordering.insert(
				candscore(pos->get_candidate_num(),
					candidates_ranked_below));
		} else {
			output_ordering.insert(
				candscore(pos->get_candidate_num(),
					-1));
		}
	}

	return {output_ordering, false};
}