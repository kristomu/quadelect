// The actual Offense-Defense model of Govan et al, but with multiplication by
// tanh instead of division.

// You could add lots of sigmoid functions here.

#ifndef _VOTE_P_ODM_TANH
#define _VOTE_P_ODM_TANH

#include "odm_gen.h"


class odm_tanh : public odm_gen {

	protected:
		double nltrans(double pairwise_value,
			double opposing_strength) const;

		// No intra-round normalization.
		void ir_norm(std::vector<double> & factors) const {}

		double get_score(double offense, double defense) const;

		std::string odm_name() const;

	public:
		odm_tanh(pairwise_type def_type_in, double tolerance_in) :
			odm_gen(def_type_in, tolerance_in) {
			update_name();
		}
};

#endif
