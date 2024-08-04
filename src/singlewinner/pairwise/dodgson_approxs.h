// Dodgson approximations: "Dodgson Quick". This is really just conddlr where
// every opposition score has been halved and rounded up. Properly defined,
// Dodgson Quick is on margins.

// MCCABE-DANSTED, John C.; PRITCHARD, Geoffrey; SLINKO, Arkadii.
// Approximability of Dodgson’s rule. Social Choice and Welfare, 2008,
// 31.2: 311-330.

#ifndef _VOTE_P_DQUICK
#define _VOTE_P_DQUICK

#include "pairwise/matrix.h"
#include "method.h"
#include "../method.h"

class dquick : public pairwise_method {
	public:
		std::pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		std::string pw_name() const {
			return ("Dodgson-Quick");
		}

		dquick(pairwise_type def_type_in):pairwise_method(def_type_in) {
			update_name();
		}

};

#endif
