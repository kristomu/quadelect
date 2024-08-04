// Kevin Venzke's Landau set implementation, used for debugging.

#pragma once

#include "singlewinner/method.h"
#include "singlewinner/pairwise/method.h"

class venzke_landau_set : public pairwise_method {
	public:
		std::pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		venzke_landau_set() : pairwise_method(CM_PAIRWISE_OPP) {}

		std::string pw_name() const {
			return "Landau (Venzke)";
		}
};
