
#ifndef _SET_ME_PDOM
#define _SET_ME_PDOM

#include "det_sets.h"

class pdom_set : public pairwise_method, private det_sets_relation {

	private:
		bool relation(const abstract_condmat & input, int a, 
				int b, const vector<bool> & hopefuls) const;

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				cache_map * cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls), false));}

		pdom_set() : pairwise_method(CM_PAIRWISE_OPP) { type_matters = false;
			update_name(); }

		string pw_name() const { return("Pareto-Nondom"); }

};

#endif
