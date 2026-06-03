#include "reverse_multiwinner.h"

std::pair<ordering, bool> reverse_multiwinner::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// Reverse the input election, excluding non-hopefuls.
	election_t reversed;

	// First get a mapping of candidate numbers by hopeful order.
	// Get the reverse mapping and the number of hopefuls too.
	// TODO? Put this in ballot_tools???

	size_t num_hopefuls = 0;
	std::vector<size_t> cand_to_hopeful(num_candidates, 0),
		hopeful_to_cand(num_candidates, 0);

	size_t hopeful_idx = 0;
	for (size_t i = 0; i < hopefuls.size(); ++i) {
		if (!hopefuls[i]) {
			continue;
		}
		cand_to_hopeful[i] = hopeful_idx;
		hopeful_to_cand[hopeful_idx] = i;
		++hopeful_idx;
		++num_hopefuls;
	}

	for (const ballot_group & group: papers) {
		ordering reversed_ballot;
		for (ordering::const_iterator pos = group.contents.begin();
			pos != group.contents.end(); ++pos) {

			if (!hopefuls[pos->get_candidate_num()]) {
				continue;
			}

			size_t hopeful_idx = cand_to_hopeful[
			pos->get_candidate_num()];

			reversed_ballot.insert(candscore(hopeful_idx,
					-pos->get_score()));
		}

		if (!reversed_ballot.empty()) {
			reversed.push_back(ballot_group(
					group.get_weight(), reversed_ballot,
					group.complete, group.rated));
		}
	}

	// Elect n-1 of n candidates.
	council_t council = base_method->get_council(
			num_hopefuls-1, num_hopefuls, reversed);

	// Turn the council output back into ordinary candidate indices
	// (undoing the hopefuls-only transformation).

	std::vector<bool> was_elected(hopefuls.size(), false);

	for (size_t hopeful_cand_num: council) {
		was_elected[hopeful_to_cand[hopeful_cand_num]] = true;
	}

	// Create the output ordering that ranks the candidate that
	// *wasn't* elected by the multiwinner method first, and
	// everybody else second.

	ordering winner_order;

	for (size_t cand = 0; cand < hopefuls.size(); ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		if (was_elected[cand]) {
			winner_order.insert(candscore(cand, 0));
		} else {
			winner_order.insert(candscore(cand, 1));
		}
	}

	return std::pair<ordering, bool>(winner_order, true);
}