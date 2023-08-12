#include "chain_climbing.h"

// For each way to add a loser to the current chain:
//	Check if it beats everybody currently in the chain
//	If so:
//		-	Remove that loser from the remaining base ordering
//		-	Add it to the top of the chain
//		-   Recurse.
//	Otherwise:
//		-	Add the current head of the chain to the list of winners.

// I initially wanted to set up a way to get a complete ordering,
// where the second placers would be those who would've won if
// the winners hadn't; but I couldn't make a good recursive method
// for it. Better get *something* done.

void chain_climbing::determine_winners(
	const condmat & condorcet_matrix, const std::vector<bool> & hopefuls,
	std::vector<bool> & winners_so_far, ordering remaining_base_ordering,
	ordering current_chain) const {

	// Get every current loser.

	std::list<candscore> losers = ordering_tools::get_loser_candscores(
			remaining_base_ordering);

	int chains_extended = 0, candidates_tested = 0;

	for (const candscore & loser: losers) {

		if (!hopefuls[loser.get_candidate_num()]) {
			continue;
		}

		++candidates_tested;

		// Check if the proposed new entrant beats everybody
		// still on the chain.
		bool beats_whole_chain = true;

		for (const candscore & chain_member : current_chain) {
			beats_whole_chain &= condorcet_matrix.beats(
					loser.get_candidate_num(),
					chain_member.get_candidate_num());
		}

		if (!beats_whole_chain) {
			continue;
		}

		++chains_extended;

		// Using the current chain size as a score ensures
		// that the new member will be ranked above everybody
		// else. We need this in order to identify the winners.
		candscore new_chain_top(loser.get_candidate_num(),
			current_chain.size());

		remaining_base_ordering.erase(loser);
		current_chain.insert(new_chain_top);

		determine_winners(condorcet_matrix, hopefuls,
			winners_so_far, remaining_base_ordering,
			current_chain);

		remaining_base_ordering.insert(loser);
		current_chain.erase(new_chain_top);
	}

	// If the remaining ordering is empty or at least one of the
	// candidates proposed to extend the chain with failed, that
	// means that the current chain top (if any) is a winner.

	// In the former case, we've run out of candidates so whoever
	// was added to the top last wins. In the latter case, there's
	// a candidate who can't be added to the chain, which makes the
	// current top the champion.

	if (chains_extended < candidates_tested || candidates_tested == 0) {
		if (current_chain.empty()) {
			return;
		}
		winners_so_far[
		current_chain.begin()->get_candidate_num()] = true;
	}
}

std::vector<bool> chain_climbing::get_winners(
	const std::list<ballot_group> & election,
	const std::vector<bool> & hopefuls,
	ordering base_method_ordering) const {

	size_t num_candidates = hopefuls.size();

	std::vector<bool> winners(num_candidates, false);
	condmat condorcet_matrix(election, num_candidates,
		CM_PAIRWISE_OPP);

	determine_winners(condorcet_matrix, hopefuls,
		winners, base_method_ordering, ordering());

	return winners;
}

// Lifted from meta/benham.

std::pair<ordering, bool> chain_climbing::elect_inner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	ordering base_outcome = base_method->elect(papers, hopefuls,
			num_candidates, cache, false);

	std::vector<bool> is_winner = get_winners(papers,
			hopefuls, base_outcome);

	double top_base_score = base_outcome.begin()->get_score();

	ordering chain_winner_ordering;

	for (int i = 0; i < num_candidates; ++i) {
		// Add all the winners with a score higher than the max
		// score by the base method.
		if (is_winner[i]) {
			chain_winner_ordering.insert(
				candscore(i, top_base_score+1));
		}
	}

	// Add all the non-winners in base method order.
	for (const candscore & base_candscore: base_outcome) {
		if (!is_winner[base_candscore.get_candidate_num()]) {
			chain_winner_ordering.insert(base_candscore);
		}
	}

	return std::pair<ordering, bool>(chain_winner_ordering, false);
}