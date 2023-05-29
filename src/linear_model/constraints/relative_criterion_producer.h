#pragma once

// This returns a bunch of relative criteria based on the caller's specs.

#include "relative_criterion.h"
#include <memory>
#include <string>

class relative_criterion_producer {
	public:
		// Return all known relative criteria with both before and after
		// number of candidates somewhere in [min_num_cands, max_num_cands].
		// If different_scenarios_only is true, skip anything below number
		// of canidates = 4 because these can be tested over a single
		// scenario (and setting it to true assumes that the algorithm list
		// inputs have been pre-sifted that way).
		std::vector<std::shared_ptr<relative_criterion_const> >
		get_all(int min_num_cands, int max_num_cands,
			bool different_scenarios_only) const;

		std::vector<std::shared_ptr<relative_criterion_const> >
		get_criteria(int min_num_cands, int max_num_cands,
			bool different_scenarios_only,
			const std::vector<std::string> & desired_criteria) const;
};
