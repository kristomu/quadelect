#include "benham.h"

#include "../../pairwise/matrix.h"

// For each way to remove a loser:
//	Remove that loser
//	If there's now a CW, add that CW to the set of winners.
//  Otherwise, recurse.

// We want to find what candidates need to be eliminated from
// the end towards the beginning in order to get a Condorcet winner
// among the remaining candidates. But because we want to find every
// candidate who could become a winner by breaking ties in some way or
// other, we need to do this recursively. The recursive procedure works
// like this:

// - If there's a CW among the remaining candidates,
//		add that candidate to the winner set and exit.
// - Otherwise, for each way to remove a loser from the
//		ordering,
//		Remove that loser and recurse.
void benham_meta::determine_winners(
	const condmat & condorcet_matrix,
	std::vector<bool> & remaining_candidates,
	std::vector<bool> & winners_so_far,
	ordering remaining_ordering) const {

	int CW = condorcet.get_CW(condorcet_matrix,
			remaining_candidates);

	if (CW != -1) {
		winners_so_far[CW] = true;
		return;
	}

	std::list<candscore> losers = ordering_tools::get_loser_candscores(
			remaining_ordering);

	for (const candscore & loser: losers) {

		remaining_ordering.erase(loser);
		remaining_candidates[loser.get_candidate_num()] = false;

		determine_winners(condorcet_matrix, remaining_candidates,
			winners_so_far, remaining_ordering);

		remaining_ordering.insert(loser);
		remaining_candidates[loser.get_candidate_num()] = true;
	}
}

std::vector<bool> benham_meta::get_winners(
	const election_t & election,
	std::vector<bool> hopefuls,
	ordering base_method_ordering) const {

	size_t num_candidates = hopefuls.size();

	std::vector<bool> winners(num_candidates, false);
	condmat condorcet_matrix(election, num_candidates, CM_PAIRWISE_OPP);

	determine_winners(condorcet_matrix, hopefuls,
		winners, base_method_ordering);

	return winners;
}

std::pair<ordering, bool> benham_meta::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	ordering base_outcome = base_method->elect(papers, hopefuls,
			num_candidates, cache, false);

	std::vector<bool> is_winner = get_winners(papers,
			hopefuls, base_outcome);

	double top_base_score = base_outcome.begin()->get_score();

	ordering benham_ordering;

	for (int i = 0; i < num_candidates; ++i) {
		// Add all the winners with a score higher than the max
		// score by the base method.
		if (is_winner[i]) {
			benham_ordering.insert(candscore(i, top_base_score+1));
		}
	}

	// Add all the non-winners in base method order.
	for (const candscore & base_candscore: base_outcome) {
		if (!is_winner[base_candscore.get_candidate_num()]) {
			benham_ordering.insert(base_candscore);
		}
	}

	return std::pair<ordering, bool>(benham_ordering, false);
}