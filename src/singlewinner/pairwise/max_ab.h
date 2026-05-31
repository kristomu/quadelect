#pragma once

#include "pairwise/matrix.h"
#include "method.h"
#include "../method.h"

#include <iostream>
#include <list>

// Implementing max A>B, which elects the candidate on the
// winning side of the strongest pairwise contest. In mathematical
// terms, A's score is max over other candidates B: A>B. Hence
// the name "max A>B" or max_ab.

// The ordinary minmax version (which only looks at the strongest
// pairwise contest of each candidate) should have no burial
// incentive. The method implemented here is the Ext-version,
// which does have *some* burial incentive due to the tiebreak,
// depending on the number of voters involved.

// Except for being a pairwise method with very low burial incentive,
// it is unremarkable; it's clearly not cloneproof, for instance.

class max_ab : public pairwise_method {

	public:
		max_ab(pairwise_type def_type_in) :
			pairwise_method(def_type_in) {
			update_name();
		}
		max_ab() :
			pairwise_method(CM_PAIRWISE_OPP) {
			update_name();
		}
		std::string pw_name() const {
			return "Ext-Max(A>/B)";
		}

		std::pair<ordering, bool> pair_elect(const abstract_condmat & iput,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;
};
