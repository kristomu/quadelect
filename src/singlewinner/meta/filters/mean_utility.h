#pragma once

// Mean-utility Approval guideline: approve (rate 1) every candidate above mean
// utility, disapprove (rate 0) everybody else.

#include "singlewinner/method.h"
#include "common/ballots.h"
#include "tools/tools.h"

#include <memory>

class mean_utility : public election_method {

	private:
		std::shared_ptr<const election_method> forward_to;

	protected:
		std::pair<ordering, bool> elect_inner(const election_t &
			papers, const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

	public:

		// the use of an integer input is intentional. The casting to
		// a double makes renormalization easier in the main body of
		// the code.
		mean_utility(std::shared_ptr<const election_method> forward_in,
			int max_in) {
			forward_to = forward_in;
		}

		std::string name() const {
			return "Mean utility Approval-[" + forward_to->name() + "]";
		}
};