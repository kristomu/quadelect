
// CDTT and CGTT, the Smith and Schwartz equivalents for the majority relation
// (instead of the beats relation). The names are short for "Condorcet, doubly
//  augmented top tier" and "Condorcet (gross) top tier".

#ifndef _SET_ME_CTT
#define _SET_ME_CTT

#include "det_sets.h"

class cdtt_set : public pairwise_method, private det_sets_relation {
	private:
		bool relation(const abstract_condmat & input, int a, int b,
				const vector<bool> & hopefuls) const {
			return(input.get_magnitude(a, b, hopefuls) > input.
					get_num_voters() * 0.5);
		}
		
	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map * cache, bool winner_only) const {
			return(pair<ordering,bool>(nested_sets(input,
							hopefuls), false));}

		cdtt_set() : pairwise_method(CM_WV) { update_name(); }

		string pw_name() const { return("CDTT"); }
};

class cgtt_set : public pairwise_method, private det_sets_relation {
        private:
                bool relation(const abstract_condmat & input, int a, int b,
                                const vector<bool> & hopefuls) const {
                        return(input.get_magnitude(a, b, hopefuls) >= input.
                                        get_num_voters() * 0.5);
                }

        public:
                pair<ordering, bool> pair_elect(const abstract_condmat & input,
                                const vector<bool> & hopefuls,
                                cache_map * cache, bool winner_only) const {
                        return(pair<ordering,bool>(nested_sets(input,
                                                        hopefuls), false));}

                cgtt_set() : pairwise_method(CM_WV) { update_name(); }

                string pw_name() const { return("CGTT"); }
};

#endif
