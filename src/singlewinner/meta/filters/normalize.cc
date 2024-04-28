#include "normalize.h"

std::pair<ordering, bool> normalize::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	// Create the derived ballots.
	election_t derived_papers;

	for (const ballot_group & g: papers) {
		ballot_group out;
		out.set_weight(g.get_weight());

		auto highest_hopeful = g.contents.begin();
		auto lowest_hopeful = g.contents.rbegin();

		// Search down the ballot to find the highest scored
		// hopeful candidate.
		while (highest_hopeful != g.contents.end() &&
			!hopefuls[highest_hopeful->get_candidate_num()]) {
			++highest_hopeful;
		}

		// If the ballot is exhausted, just pass it along: it doesn't
		// matter that it's out of spec because the method we forward
		// it to should ignore it.
		if (highest_hopeful == g.contents.end()) {
			derived_papers.push_back(g);
			continue;
		}

		while (lowest_hopeful != g.contents.rend() &&
			!hopefuls[lowest_hopeful->get_candidate_num()]) {
			++lowest_hopeful;
		}

		assert(lowest_hopeful != g.contents.rend());

		double ballot_minimum = lowest_hopeful->get_score(),
			   ballot_maximum = highest_hopeful->get_score();

		for (const candscore & cs: g.contents) {
			if (!hopefuls[cs.get_candidate_num()]) {
				continue;
			}

			double score = round(renorm(ballot_minimum,
						ballot_maximum, cs.get_score(),
						0.0, maximum));

			out.contents.insert(candscore(
					cs.get_candidate_num(), score));
		}
		derived_papers.push_back(out);
	}

	return forward_to->elect_detailed(derived_papers,
			hopefuls, num_candidates, cache, winner_only);
}