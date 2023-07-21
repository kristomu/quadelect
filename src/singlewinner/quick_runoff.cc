#include "quick_runoff.h"

#include "../pairwise/matrix.h"
#include "positional/simple_methods.h"

ordering quick_runoff::winner_to_ordering(size_t winner,
	size_t num_candidates) const {

	ordering winner_order;

	for (size_t cand = 0; cand < num_candidates; ++cand) {
		if (cand == winner) {
			winner_order.insert(candscore(cand, 1));
		} else {
			winner_order.insert(candscore(cand, 0));
		}
	}

	return winner_order;
}

std::pair<ordering, bool> quick_runoff::elect_inner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	plurality plurality_eval(PT_FRACTIONAL);

	double numvoters = 0;
	std::list<ballot_group>::const_iterator pos;
	for (pos = papers.begin(); pos != papers.end(); ++pos) {
		numvoters += pos->get_weight();
	}

	ordering plurality_outcome = plurality_eval.elect(papers, hopefuls,
			num_candidates, cache, false);

	condmat pairwise_matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	// Starting at the first candidate, check if he pairwise loses by a
	// majority to the next candidate. If so, keep going, otherwise return
	// that candidate at top and everybody else at bottom.

	for (auto pos = plurality_outcome.begin();
		pos != plurality_outcome.end(); ++pos) {

		if (!hopefuls[pos->get_candidate_num()]) {
			continue;
		}

		// If we're the last candidate in the order, then
		// we always win, so set the magnitude of our victory
		// to the maximum possible.
		double beats_next_by = numvoters;

		auto next_in_line = std::next(pos);

		// Skiop non-hopefuls.
		while (!hopefuls[next_in_line->get_candidate_num()] &&
			next_in_line != plurality_outcome.end()) {
			++next_in_line;
		}

		if (next_in_line != plurality_outcome.end()) {
			beats_next_by = pairwise_matrix.get_magnitude(
					pos->get_candidate_num(),
					next_in_line->get_candidate_num());
		}

		// If the candidate next in line doesn't beat us by a
		// majority, we win.
		if (2 * beats_next_by >= numvoters) {
			return std::pair<ordering, bool>(
					winner_to_ordering(pos->get_candidate_num(),
						num_candidates), true);
		}
	}

	// The loop above should always exit the function, so
	// if we get here, something's wrong.
	assert(false);
}