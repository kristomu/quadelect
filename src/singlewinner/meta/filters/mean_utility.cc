
#include "mean_utility.h"

std::pair<ordering, bool> mean_utility::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	// Create the derived ballots.
	election_t derived_papers;

	for (const ballot_group & g: papers) {
		ballot_group out;
		out.set_weight(g.get_weight());

		// Get the mean utility threshold
		double utilities = 0;
		int entries = 0;

		for (const candscore & cs: g.contents) {
			if (!hopefuls[cs.get_candidate_num()]) {
				continue;
			}

			utilities += cs.get_score();
			++entries;
		}

		// If we're dealing with an exhausted ballot, we can't
		// normalize. However, the method we're passing the
		// exhausted ballot to shouldn't care about the scores
		// being out of scope - so we can just pass the ballot
		// through unchanged.

		if (entries == 0) {
			derived_papers.push_back(g);
			continue;
		}

		double mean_utility = utilities/entries;
		if (!isfinite(mean_utility)) {
			throw std::invalid_argument("Mean utility thresholding:"
				" mean utility is not finite!");
		}


		for (const candscore & cs: g.contents) {
			double score = 0;
			if (cs.get_score() > mean_utility) {
				score = 1;
			}

			out.contents.insert(candscore(
					cs.get_candidate_num(), score));
		}
		derived_papers.push_back(out);

	}

	return forward_to->elect_detailed(derived_papers,
			hopefuls, num_candidates, cache, winner_only);
}