// The Hypertext Induced Text Search (HITS) algorithm.

#ifndef _VOTE_P_ODM_HITS
#define _VOTE_P_ODM_HITS

// HITS works by having two score factors: hub and authority. Hub factors are
// equal to the sum of authority factors of all pages the hub links to, and so
// is equivalent to Offense, while Authority factors are equal to the sum of
// hub factors of all pages that link to the authority, and so is equivalent to
// Defense.

// In a voting context, this means that the transformation function is simply
// multiplication with the other factor. This would easily go out of bounds,
// so the factors need to be contracted (normalized) after calculation, which
// is done as by Wikipedia's sum of squares suggestion.

#include "odm_gen.h"

using namespace std;

class hits : public odm_gen {

	protected:
		double nltrans(double pairwise_value,
			double opposing_strength) const;

		void ir_norm(vector<double> & factors) const;

		double get_score(double offense, double defense) const;

		string odm_name() const;

	public:
		hits(pairwise_type def_type_in, double tolerance_in) :
			odm_gen(def_type_in, tolerance_in) {
			update_name();
		}
};

#endif
