// Random Pair. This random method picks two candidates at random and elects
// the winner of the two. If they're tied, the method simply flips a coin.

// Random Pair is strategy-proof with respect to the voters, but not with
// respect to agenda manipulation - it is not cloneproof.

// BLUESKY: Do a random pair MAM type thing to get a full ordering. That is,
// take a list of all possible candidate pairs then jumble them up, then proceed
// as in MAM/Ranked Pairs.

// For now, we simply repeat the procedure with the winner excluded until we
// have a full rank.

#ifndef _VOTE_P_RANDPAIR
#define _VOTE_P_RANDPAIR

#include "../../../pairwise/matrix.h"
#include "../method.h"
#include "../../method.h"

#include <assert.h>
#include <iostream>
#include <list>

class randpair : public pairwise_method {
	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		string pw_name() const {
			return ("Random Pair");
		}

		// Like Copeland, the type doesn't matter since we're only
		// looking at whether X beats Y.
		randpair(pairwise_type def_type_in) :
			pairwise_method(def_type_in) {
			type_matters = false;
			update_name();
		}
};

#endif
