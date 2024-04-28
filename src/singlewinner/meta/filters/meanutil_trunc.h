#pragma once

// Mean-utility truncation: pass through every score above mean utility,
// and remove everything at or below mean utility. For DAC (though DAC
// itself needs to be implemented first).

// TODO??? Unify this with mean_utility? How?

#include "../../method.h"
#include "../../../ballots.h"
#include "../../../tools/tools.h"

#include <memory>

class mean_utility_trunc : public election_method {

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
		mean_utility_trunc(std::shared_ptr<const election_method> forward_in,
			int max_in) {
			forward_to = forward_in;
		}

		std::string name() const {
			return "Mean utility truncation-[" + forward_to->name() + "]";
		}
};