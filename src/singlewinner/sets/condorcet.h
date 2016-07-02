// This set method produces the "Condorcet set", which is the CWs ranked in
// order if there is a CW, otherwise every member is equally ranked.

#ifndef _SET_CONDORCET
#define _SET_CONDORCET

#include "../method.h"
#include "../pairwise/method.h"

class condorcet_set : public pairwise_method {

	private:
		// Returns -1 if none.
		int get_CW(const abstract_condmat & input, 
				const vector<bool> & hopefuls) const;

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				cache_map & cache, bool winner_only) const;

		condorcet_set() : pairwise_method(CM_WV) { 
			type_matters = false; update_name(); }

		string pw_name() const { return("Condorcet"); }

};

#endif
