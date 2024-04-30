#include "clamp.h"
#include <iostream>

std::pair<ordering, bool> clamp::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	// Create the derived ballots.
	election_t derived_papers;

	for (const ballot_group & g: papers) {
		ballot_group out;
		out.set_weight(g.get_weight());

		for (const candscore & cs: g.contents) {
			if (!hopefuls[cs.get_candidate_num()]) {
				continue;
			}

			double score = round(renorm(minimum_in,
						maximum_in, cs.get_score(),
						0.0, maximum_out));

			// Clamp if required.
			score = std::min(maximum_out, std::max(0.0, score));

			out.contents.insert(candscore(
					cs.get_candidate_num(), score));
		}
		derived_papers.push_back(out);
	}

	return forward_to->elect_detailed(derived_papers,
			hopefuls, num_candidates, cache, winner_only);
}