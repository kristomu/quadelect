
#ifndef _VOTE_P_EXPERIMENTAL
#define _VOTE_P_EXPERIMENTAL

#include "pairwise/matrix.h"
#include "method.h"
#include "../method.h"

#include <iostream>
#include <list>

class minmaxy_experimental : public pairwise_method {

	public:
		// HACK fix later
		minmaxy_experimental(pairwise_type def_type_in) :
			pairwise_method(CM_PAIRWISE_OPP) {
			update_name();
		}
		minmaxy_experimental() :
			pairwise_method(CM_PAIRWISE_OPP) {
			update_name();
		}
		std::string pw_name() const {
			return ("Minmaxy-Experimental");
		}

		std::pair<ordering, bool> pair_elect(const abstract_condmat & iput,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;
};

#endif
