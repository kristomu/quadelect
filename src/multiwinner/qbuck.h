#pragma once

// "Quota Bucklin" (QLTD PR variants)

// Oddly, we get results that are worse than STV, while sec gives results better
// than STV. Perhaps we should try to implement a direct copy of sec (or its
// method, at least), verify it gives the correct results, and then see if it
// does better than STV.

#include "tools/tools.h"
#include "common/ballots.h"
#include "singlewinner/positional/aggregator.h"
#include "singlewinner/positional/positional.h"
#include "singlewinner/positional/simple_methods.h"
#include "methods.h"

#include <vector>
#include <list>

class qltd_pr : public multiwinner_method {
	private:
		bool hare_quota;
		bool restart_at_zero;
		bool bucklin;

		candscore get_first_above_quota(
			const std::vector<std::vector<double> > & positional_matrix,
			const std::vector<bool> & hopefuls, const double quota,
			double start_at, double & surplus) const;
		bool is_contributing(const ballot_group & ballot,
			const std::vector<bool> & hopefuls,
			size_t to_this_candidate,
			double num_preferences) const;

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		std::string name() const;

		qltd_pr(bool use_hare_quota, bool use_bucklin,
			bool restart_after_elect) {
			hare_quota = use_hare_quota; bucklin = use_bucklin;
			restart_at_zero = restart_after_elect;
		}
};