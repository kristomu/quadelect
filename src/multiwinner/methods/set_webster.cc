
#include "set_webster.h"

exhaustive_optima set_webster::get_opt(double proposed_factor,
	size_t council_size, size_t num_candidates,
	const election_t & ballots) const {

	std::vector<size_t> v(num_candidates);
	std::iota(v.begin(), v.end(), 0);

	set_webster_helper swh(proposed_factor);

	swh.process_ballots(ballots, num_candidates);

	exhaustive_optima optimum = for_each_combination(v.begin(),
			v.begin() + council_size, v.end(), swh);

	return optimum;
}

council_t set_webster::get_council(size_t council_size,
	size_t num_candidates, const election_t & ballots) const {

	double num_voters = ballot_tools::get_num_voters(ballots);

	// The factor must be somewhere between these endpoints.
	double min_factor = 0, max_factor = 2 * council_size/num_voters;
	bool sat_min = true;//, sat_max = false;

	exhaustive_optima opt_middle(true);

	// Do the bisection search.
	while (max_factor - min_factor > 1e-6) {
		double factor = (min_factor + max_factor)/2.0;

		opt_middle = get_opt(factor, council_size, num_candidates, ballots);

		// Positive means success, negative is failure.
		bool sat_middle = opt_middle.get_optimum() > 0;

		if (sat_middle == sat_min) {
			min_factor = factor;
			sat_min = sat_middle;
		} else {
			max_factor = factor;
			//sat_max = sat_middle;
		}
	}

	// TODO: This only picks "a" solution. We don't know if it's unique.
	// Do something about it. And about this ugly conversion.

	std::vector<size_t> om = opt_middle.get_optimal_solution();
	council_t output;
	std::copy(om.begin(), om.end(), std::back_inserter(output));

	return output;
}
