// Warren D. Smith's Sinkhorn method.

#ifndef _VOTE_P_SINKHORN
#define _VOTE_P_SINKHORN

#include "pairwise/matrix.h"
#include "method.h"
#include "../method.h"

#include <iostream>

// Like Keener, tolerance specifies the error before convergence is assumed.
// While Sinkhorn converges to 1, to keep compatible with Keener, the input
// parameter has 0 as perfect convergence, just like Keener.

// Add_one adds 1 to all the pairwise matrix entries so that the method
// converges quickly. WDS also considers that more likely to give a good result.

struct sinkhorn_factor {
	std::vector<double> row;
	std::vector<double> col;
};

class sinkhorn : public pairwise_method {
	private:
		double tolerance;
		bool add_one;

		sinkhorn_factor get_sinkhorn_factor(int max_iterations, const
			std::vector<std::vector<double > > & input_matrix,
			bool debug) const;

	public:
		sinkhorn(pairwise_type def_type_in, double tolerance_in,
			bool add_one_in) : pairwise_method(def_type_in) {
			tolerance = tolerance_in; add_one = add_one_in;
			update_name();
		}

		std::pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		std::string pw_name() const;

		void set_tolerance(double tolerance_in) {
			tolerance = tolerance_in; update_name();
		}
};

#endif
