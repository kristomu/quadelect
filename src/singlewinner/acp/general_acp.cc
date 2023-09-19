#include "general_acp.h"

#include "../../pairwise/matrix.h"

std::pair<ordering, bool> generalized_acp::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	ordering base_outcome = base_method->elect(papers, hopefuls,
			num_candidates, cache, false);

	// Get the base method winners.
	std::list<int> base_winners = ordering_tools::get_winners(
			base_outcome);

	// Now create a derived matrix for each base winner, and
	// gather the results: both as a boolean vector (true = was
	// winner, false = otherwise), and as an election in its own
	// right. I may use the later for passing through some other
	// election method as a different approach to determining the
	// ultimate winners.

	std::vector<bool> was_pairwise_winner(num_candidates, false);
	election_t pairwise_outcomes_election;

	for (int base_winner: base_winners) {

		// Truncate after the base method winner and create
		// a Condorcet matrix based on this.
		election_t truncated_papers =
			ballot_tools::truncate_after(papers, base_winner);

		condmat pairwise_matrix(truncated_papers, num_candidates, CM_WV);

		ordering pw_outcome = pairwise_base->pair_elect(pairwise_matrix,
				hopefuls, false).first;

		// Break ties by the base method.
		pw_outcome = ordering_tools::ranked_tiebreak(pw_outcome,
				base_outcome, num_candidates);

		// Break further ties in favor of the base winner.
		// TODO: this is essentially winner_to_ordering. Put it into
		// ordering_tools.
		ordering favors_base;
		for (int i = 0; i < num_candidates; ++i) {
			if (i == base_winner) {
				favors_base.insert(candscore(i, 1));
			} else {
				favors_base.insert(candscore(i, 0));
			}
		}

		pw_outcome = ordering_tools::ranked_tiebreak(pw_outcome,
				favors_base, num_candidates);

		std::list<int> pw_winners = ordering_tools::get_winners(
				pw_outcome);

		for (int pw_winner: pw_winners) {
			was_pairwise_winner[pw_winner] = true;
		}

		pairwise_outcomes_election.push_back(ballot_group(
				1, pw_outcome, true, false));
	}

	ordering ordering_out;

	for (int i = 0; i < num_candidates; ++i) {
		if (!hopefuls[i]) {
			continue;
		}

		if (was_pairwise_winner[i]) {
			ordering_out.insert(candscore(i, 1));
		} else {
			ordering_out.insert(candscore(i, 0));
		}
	}

	return std::pair<ordering, bool>(
			ordering_out, false);
}
