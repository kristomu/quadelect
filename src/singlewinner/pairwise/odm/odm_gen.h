#ifndef _VOTE_P_ODM_GEN
#define _VOTE_P_ODM_GEN

#include "../../../pairwise/matrix.h"
#include "../method.h"
#include "../../method.h"

#include <assert.h>
#include <iostream>

using namespace std;

// Generic class for methods similar to the Offense-Defense model of Govan et 
// al. These methods are Sinkhorn-esque: they involve determining row and column
// factors depending on each other and on either the rows or columns of the
// pairwise matrix. The calculation of the row and column factors is alternated,
// and the score ultimately converges.

class odm_gen : public pairwise_method {

	private:
		double tolerance;

	protected:
		// Transformation to determine new factors. 
		virtual double nltrans(double pairwise_value, 
				double opposing_strength) const = 0;

		// Intra-round normalization
		virtual void ir_norm(vector<double> & factors) const = 0;

		// Score function.
		virtual double get_score(double offense, 
				double defense) const = 0;

		virtual string odm_name() const = 0;

	public:
		odm_gen(pairwise_type def_type_in, double tolerance_in) : 
		 pairwise_method(def_type_in){
			tolerance = tolerance_in;
		}

		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map & cache, bool winner_only) const;

		string pw_name() const;

		void set_tolerance(double tolerance_in) { 
			tolerance = tolerance_in; update_name(); }
};

#endif
