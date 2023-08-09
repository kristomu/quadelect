#include "first_seen.h"

std::pair<ordering, bool> first_seen_tiebreak::elect_inner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// This vector contains, for each candidate, the
	// highest rank that candidate obtained got ranked on
	// some ballot. The rank indices are negated so that
	// a higher value corresponds to better rank, so top
	// rank is 0, second place -1, and so on.
	std::vector<int> rank_first_seen(num_candidates,
		num_candidates+1);

	for (const ballot_group & ballot: papers) {

		ordering ballot_order;

		// Dump everything that's hopeful into the ballot order.
		for (const candscore & cs: ballot.contents) {
			if (hopefuls[cs.get_candidate_num()]) {
				ballot_order.insert(cs);
			}
		}

		ordering ranked_order = ordering_tools().
			scrub_scores(ballot_order);

		// Now everything in first place has score 0, everything
		// in second has score -1 and so on.

		for (const candscore & cs: ranked_order) {
			rank_first_seen[cs.get_candidate_num()] =
				std::max(rank_first_seen[cs.get_candidate_num()],
					(int)cs.get_score());
		}
	}

	// Now it's a simple matter of using the rank_first_seen as a
	// score array.

	ordering outcome;

	for (int i = 0; i < num_candidates; ++i) {
		if (hopefuls[i]) {
			outcome.insert(candscore(i, rank_first_seen[i]));
		}
	}

	return (std::pair<ordering, bool>(outcome, false));
}