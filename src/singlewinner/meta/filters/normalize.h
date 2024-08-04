#pragma once

// Linear normalization: each voter rates his least favorite a zero and greatest
// favorite n, so that the ballots maximally use the integers [0, ..., n].

#include "singlewinner/method.h"
#include "common/ballots.h"
#include "tools/tools.h"

#include <memory>

class normalize : public election_method {

	private:
		std::shared_ptr<const election_method> forward_to;
		double maximum;

	protected:
		std::pair<ordering, bool> elect_inner(const election_t &
			papers, const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

	public:

		// the use of an integer input is intentional. The casting to
		// a double makes renormalization easier in the main body of
		// the code.
		normalize(std::shared_ptr<const election_method> forward_in,
			int max_in) {
			forward_to = forward_in;
			maximum = max_in;
		}

		std::string name() const {
			return "Normalize(" + itos(maximum) + ")-[" + forward_to->name() + "]";
		}
};