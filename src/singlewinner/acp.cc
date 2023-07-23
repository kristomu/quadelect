#include "acp.h"

#include "../pairwise/matrix.h"
#include "positional/simple_methods.h"

int adjusted_cond_plur::get_adjusted_winner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls,
	size_t plurality_winner, size_t num_candidates) const {

	std::list<ballot_group> truncated_papers;

	for (const ballot_group & group: papers) {
		ordering truncated_ballot;
		bool found_original_winner = false;
		for (ordering::const_iterator pos = group.contents.begin();
			pos != group.contents.end() && !found_original_winner;
			++pos) {
			truncated_ballot.insert(*pos);
			found_original_winner |= (
					pos->get_candidate_num() == plurality_winner);
		}

		bool ballot_is_complete = (
				truncated_ballot.size() == num_candidates);

		truncated_papers.push_back(ballot_group(
				group.get_weight(), truncated_ballot, ballot_is_complete,
				group.rated));
	}

	// Create a Condorcet matrix for the derived ballot set.
	condmat pairwise_matrix(papers, num_candidates, CM_WV);

	int condorcet_winner = condorcet.get_CW(
			pairwise_matrix, hopefuls);

	if (condorcet_winner == -1) {
		return plurality_winner;
	} else {
		return condorcet_winner;
	}
}

std::pair<ordering, bool> adjusted_cond_plur::elect_inner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	plurality plurality_eval(PT_FRACTIONAL);

	double numvoters = 0;
	std::list<ballot_group>::const_iterator ballot_ref;
	for (ballot_ref = papers.begin(); ballot_ref != papers.end();
		++ballot_ref) {
		numvoters += ballot_ref->get_weight();
	}

	ordering plurality_outcome = plurality_eval.elect(papers, hopefuls,
			num_candidates, cache, false);

	condmat pairwise_matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	// Get the Plurality winners.
	std::vector<int> plur_winners;
	for (ordering::const_iterator pos = plurality_outcome.begin();
		pos != plurality_outcome.end() &&
		pos->get_candidate_num() == plurality_outcome.begin()->get_candidate_num();
		++pos) {

		plur_winners.push_back(pos->get_candidate_num());
	}

	// Now call the get_adjusted_winners function for all such Plurality
	// winners and mark them off in the seen_candidate vector. Every
	// candidate in that vector thus gets a score of 1, and everybody else
	// a score of zero. (May change this later.)

	std::vector<bool> seen_candidate(num_candidates, false);

	for (int plurality_winner: plur_winners) {
		int adjusted_winner = get_adjusted_winner(
				papers, hopefuls, plurality_winner, num_candidates);

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