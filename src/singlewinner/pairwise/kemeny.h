// Kemeny. This method is, unfortunately, NP-hard. We do as best as we can by
// making use of the GLPK to solve it through integer programming. (We might
// handle the linear programming relaxation too, later: just solve and then
// output the X>Y weights into Ranked Pairs.)

// The exact integer program will be given in comments within the pair_elect
// function itself.

// YOUNG, H. Peyton; LEVENGLICK, Arthur. A consistent extension of
// Condorcet’s election principle. SIAM Journal on applied Mathematics,
// 1978, 35.2: 285-300.

#ifndef _VOTE_P_KEMENY
#define _VOTE_P_KEMENY

#include "../../pairwise/matrix.h"
#include "method.h"

#include <iostream>
#include <vector>

using namespace std;

class kemeny : public pairwise_method {
	private:
		vector<vector<bool> > solve_kemeny(
			const abstract_condmat & input,
			const vector<bool> & hopefuls,
			bool debug) const;

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		string pw_name() const {
			return ("Kemeny");
		}

		kemeny(pairwise_type def_type_in):pairwise_method(def_type_in) {
			update_name();
		}
};

#endif
