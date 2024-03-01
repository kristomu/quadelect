#include "acp.h"

#include "../../pairwise/matrix.h"

int adjusted_cond_plur::get_adjusted_winner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	size_t base_winner, size_t num_candidates) const {

	// Truncate after the base method winner and create
	// a Condorcet matrix based on this.
	election_t truncated_papers =
		ballot_tools::truncate_after(papers, base_winner);

	condmat pairwise_matrix(truncated_papers, num_candidates, CM_WV);

	int condorcet_winner = condorcet.get_CW(
			pairwise_matrix, hopefuls);

	if (condorcet_winner == -1) {
		return base_winner;
	} else {
		return condorcet_winner;
	}
}

std::pair<ordering, bool> adjusted_cond_plur::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	ordering base_outcome = base_method->elect(papers, hopefuls,
			num_candidates, cache, false);

	// Get the base method (usually Plurality) winners.
	std::vector<int> base_winners = ordering_tools::get_winners(
			base_outcome);

	// Now call the get_adjusted_winners function for all such Plurality
	// winners and mark them off in the seen_candidate vector. Every
	// candidate in that vector thus gets a score of 1, and everybody else
	// a score of zero. (May change this later.)

	std::vector<bool> seen_candidate(num_candidates, false);

	for (int base_winner: base_winners) {
		if (!hopefuls[base_winner]) {
			continue;
		}
		int adjusted_winner = get_adjusted_winner(papers,
				hopefuls, base_winner, num_candidates);

		seen_candidate[adjusted_winner] = true;
	}

	ordering ordering_out;

	for (int i = 0; i < num_candidates; ++i) {
		if (!hopefuls[i]) {
			continue;
		}

		if (seen_candidate[i]) {
			ordering_out.insert(candscore(i, 1));
		} else {
			ordering_out.insert(candscore(i, 0));
		}
	}

	return std::pair<ordering, bool>(
			ordering_out, false);
}
