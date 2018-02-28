// Keener Eigenvector method. Let's hope I get it right :-)

#ifndef _VOTE_P_KEENER
#define _VOTE_P_KEENER

#include "../../pairwise/matrix.h"
#include "method.h"
#include "../method.h"

#include <assert.h>
#include <iostream>

using namespace std;

// Tolerance specifies the, well, tolerance before it assumes convergence.
// Add_one adds 1 to all the Condorcet matrix entries so that the method always
// converges (and geometrically). Normalize_diagonal sets the diagonal s.th.
// U_aa = num voters - sum (b != a) U_ba for all a, which is required in order
// for the ratings to have a proper Markovian meaning.

class keener : public pairwise_method {
	private:
		double tolerance;
		bool add_one, normalize_diagonal;

	public:
		keener(pairwise_type def_type_in, double tolerance_in, bool add_one_in, bool normalize_diagonal_in) : pairwise_method(def_type_in) { tolerance = tolerance_in; add_one = add_one_in; normalize_diagonal = normalize_diagonal_in; update_name(); }
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				cache_map * cache, bool winner_only) const;

		string pw_name() const;

		void set_tolerance(double tolerance_in) { tolerance = tolerance_in; update_name(); }
};

#endif
