// The actual Offense-Defense model of Govan et al.

#ifndef _VOTE_P_ODM_ODM
#define _VOTE_P_ODM_ODM

// In this model, we first determine offense factors which are equal to the
// pairwise scores of each candidate against each other, divided by the other's
// defense score, and then defense factors which are equal to the pairwise 
// scores against candidate by all others, divided by those candidate's offense
// score. The score (offense[x]/defense[x]) converges if all values are 
// positive.

#include "odm_gen.h"

using namespace std;

class odm : public odm_gen {

	protected:
		double nltrans(double pairwise_value, 
				double opposing_strength) const;

		// No intra-round normalization.
		void ir_norm(vector<double> & factors) const {}

		double get_score(double offense, double defense) const;

		string odm_name() const;

	public:
		 odm(pairwise_type def_type_in, double tolerance_in) :
			 odm_gen(def_type_in, tolerance_in) { update_name(); }

};

#endif
