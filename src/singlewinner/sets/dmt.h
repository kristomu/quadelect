#pragma once

#include "../method.h"

// The (innermost) dominant mutual third set is defined as the
// smallest set that's a solid coalition supported by more than
// a third of the voters, and whose members all beat everybody
// outside the set. It is a superset of the Smith set.

// This class is pretty much a cut and paste of the mutual
// majority set code, since both are based around solid
// coalitions.

class dmt_set : public election_method {
	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Dominant mutual third";
		}
};