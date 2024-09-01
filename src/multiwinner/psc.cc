#include "psc.h"
#include "coalitions/coalitions.h"

#include <numeric>

// Returns the number of candidates who must be elected, given
// the total weight and support of the coalition (unless this is more
// than the number of candidates in that coalition).
size_t coalition_elimination::quota_limit(double total_weight,
	double support, size_t council_size) const {

	// The number of voters must *exceed* k Droop quotas for the
	// constraint to be k seats.

	// This means that
	//		support > k * total_weight / (council_size + 1)

	// but we need to rearrange because numerical precision is a pain.
	// So
	//		support * (council_size + 1) * k > total_weight,
	//
	// Let p = support * (council_size + 1) / total_weight. Then if
	// p is integer, the support exactly fills a Droop quota but doesn't
	// exceed it, and we should return p-1. If p is *not* an integer, then
	// we have exceeded the integer value of p, and should return it.
	// This can be handled in both cases by rounding up and subtracting
	// one.

	// The above is for Droop. We can generalize to Hare, and everything
	// in between, by varying the + 1 in the denominator, done with
	// quota_mod here.

	size_t minimum = ceil(support *
			(council_size + quota_mod) / total_weight - 1);

	return minimum;
}

std::list<int> coalition_elimination::get_council(int council_size,
	int num_candidates, const election_t & ballots) const {

	// Get the elimination order from a Condorcet method. Perhaps
	// ranked pairs would be superior here due to LIIA?
	/*ordering base_order = schulze(CM_WV).pair_elect(condmat(ballots,
		num_candidates, CM_WV), false).first;*/

	ordering base_order = base_method->elect(ballots, num_candidates,
			false);

	std::vector<coalition_data> solid_coalitions = get_solid_coalitions(
			ballots, std::vector<bool>(num_candidates, true), num_candidates);

	double num_voters = ballot_tools::get_num_voters(ballots);

	std::set<size_t> continuing;
	for (int i = 0; i < num_candidates; ++i) {
		continuing.insert(i);
	}

	for (auto pos = base_order.rbegin(); pos != base_order.rend()
		&& continuing.size() > (size_t)council_size; ++pos) {

		size_t test_candidate = pos->get_candidate_num();

		// Go through the coalitions and see if we violate any constraints.
		bool ok_to_eliminate = true;

		for (auto cc_pos = solid_coalitions.begin();
			cc_pos != solid_coalitions.end() && ok_to_eliminate; ++cc_pos) {

			// Must elect at least this many.
			size_t elect_constraint = std::min(cc_pos->coalition.size(),
					quota_limit(num_voters, cc_pos->support, council_size));

			if (elect_constraint == 0) {
				continue;
			}

			// Get candidates who are still in the running and who
			// are also in the coalition. Then remove the candidate
			// that we'd like to eliminate and see if we pass the
			// constraint.
			std::set<size_t> proposed_in_coalition;
			std::set_intersection(
				continuing.begin(), continuing.end(),
				cc_pos->coalition.begin(), cc_pos->coalition.end(),
				std::inserter(proposed_in_coalition,
					proposed_in_coalition.begin()));

			proposed_in_coalition.erase(test_candidate);

			ok_to_eliminate &=
				proposed_in_coalition.size() >= elect_constraint;
		}

		if (ok_to_eliminate) {
			continuing.erase(test_candidate);
		}
	}

	assert(continuing.size() == council_size);

	return std::list<int>(continuing.begin(), continuing.end());
}