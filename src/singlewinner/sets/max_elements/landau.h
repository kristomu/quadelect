
// The uncovered set or Landau set. This is the set of those who have length 2
// beatpaths to every candidate outside the set.

#ifndef _SET_ME_LANDAU
#define _SET_ME_LANDAU

#include "det_sets.h"

class landau_set : public pairwise_method, private det_sets_relation {
	private:
		bool relation(const abstract_condmat & input, int a, int b,
				const vector<bool> & hopefuls) const {
			return(input.get_magnitude(a, b, hopefuls) >=
					input.get_magnitude(b, a, hopefuls));
		}

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map * cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls, 2), false));}

		landau_set() : pairwise_method(CM_WV) { update_name(); }

		string pw_name() const { return("Landau"); }
};

#endif
