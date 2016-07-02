
// Code for the Schwartz set, which is like the Smith set but with beats instead
// of beats-or-ties.

#ifndef _SET_ME_SCHWARTZ
#define _SET_ME_SCHWARTZ

#include "det_sets.h"

class schwartz_set : public pairwise_method, private det_sets_relation {

	private:
		bool relation(const abstract_condmat & input, int a,
				int b, const vector<bool> & hopefuls) const {
			return(input.get_magnitude(a, b, hopefuls) >
					input.get_magnitude(b, a, hopefuls));
		}

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls), false));}

		schwartz_set() : pairwise_method(CM_WV) { type_matters = false;
			update_name(); }

		string pw_name() const { return("Schwartz"); }
};

#endif
