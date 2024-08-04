
#include "asm.h"

// We need a Condorcet matrix for the pairwise information.
#include "pairwise/matrix.h"

#include <iostream>

std::pair<ordering, bool> app_sorted_margins::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	// Use pairwise opposition for now... allow different options
	// later???
	condmat pairwise_matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	std::vector<double> approvals(num_candidates, 0);
	std::vector<std::pair<double, size_t> > approvals_by_cand;

	// Use cardinal ratings to get the scores.
	ordering approval_outcome = approval_filter->elect(
			papers, hopefuls, num_candidates, cache, false);

	for (const candscore & cs: approval_outcome) {
		if (hopefuls[cs.get_candidate_num()]) {
			approvals[cs.get_candidate_num()] = cs.get_score();
			approvals_by_cand.push_back({cs.get_score(), cs.get_candidate_num()});
		}
	}

	// Sort the candidates by approval score. (This is a bit ugly, the
	// std::pair stuff can be replaced with just storing the candidate
	// indices and using a lambda to define the comparison operator...)

	// NOTE: This strictly speaking violates neutrality.
	std::sort(approvals_by_cand.begin(), approvals_by_cand.end(),
		std::greater<>());

	size_t num_hopefuls = approvals_by_cand.size();
	bool need_sorting;

	do {
		need_sorting = false;
		double approval_margin_record = -1, approval_margin;

		// NOTE: recordholder is an index into the approvals by cand
		// list, not a candidate number!
		size_t recordholder = num_hopefuls+1;

		// Look for adjacent candidates that are out of order.
		for (size_t i = 0; i < num_hopefuls-1; ++i) {
			size_t incumbent = approvals_by_cand[i].second,
				   challenger = approvals_by_cand[i+1].second;

			if (!pairwise_matrix.beats(challenger, incumbent)) {
				continue;
			}

			approval_margin = fabs(approvals_by_cand[i].first -
					approvals_by_cand[i+1].first);

			if (!need_sorting || approval_margin <= approval_margin_record) {
				need_sorting = true;
				approval_margin_record = approval_margin;
				recordholder = i;
			}
		}

		if (need_sorting) {
			// Check that we don't still have the sentinel value (should
			// never happen).
			assert(recordholder != num_hopefuls+1);

			std::swap(approvals_by_cand[recordholder],
				approvals_by_cand[recordholder+1]);
		}
	} while (need_sorting);

	// Now the approvals by cand is in the proper order, so just dump
	// to an ordering.

	ordering out;
	size_t rank = num_hopefuls;

	for (auto & approval_cand: approvals_by_cand) {
		out.insert(candscore(approval_cand.second, rank--));
	}

	return std::pair<ordering, bool>(out, false);
}