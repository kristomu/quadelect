
// The Smith set: Minimal set of all candidates who are beaten or tied only by
// those inside the set.

#ifndef _SET_ME_SMITH
#define _SET_ME_SMITH

#include "det_sets.h"

class smith_set : public pairwise_method, private det_sets_relation {

	private:
		bool relation(const abstract_condmat & input, int a,
			int b, const vector<bool> & hopefuls) const;

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const {
			return (pair<ordering,bool>(nested_sets(input,
							hopefuls), false));
		}

		smith_set() : pairwise_method(CM_WV) {
			type_matters = false;
			update_name();
		}

		string pw_name() const {
			return ("Smith");
		}

};

#endif
