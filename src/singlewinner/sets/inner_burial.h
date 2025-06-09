#pragma once

// The "inner burial set" defined by the criterion of
// http://lists.electorama.com/pipermail/election-methods-electorama.com/2023-August/004730.html

// If no matter what candidates you eliminate, if you don't eliminate A and B,
// and A has more than 1/(number of remaining candidates) of the total support,
// and A beats B pairwise, then B is outside this set.

// Though brittle, this may be useful for burial resistance analysis. If nothing
// else, it will be useful for checking if methods pass the generalized DMTCBR
// criterion.

#include "../method.h"
#include "pairwise/matrix.h"
#include "singlewinner/dmt/resistant/subelections.h"

class inner_burial_set : public election_method {
	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Inner burial set";
		}
};