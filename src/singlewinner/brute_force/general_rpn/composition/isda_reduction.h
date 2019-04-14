#pragma once

// This is not quite a relative criterion, because it depends on
// scenarios, not just on the number of candidates. Thus it has to be
// implemented here rather than in linear_model.

#include "vector_ballot.h"

#include "../../../../linear_model/constraints/numvoters.h"
#include "../../../../linear_model/constraints/pairwise.h"
#include "../../../../linear_model/constraints/general.h"

#include <vector>

class isda_reduction {
	public:
		std::vector<bool> smith_set(const std::vector<std::vector<bool> > &
			copeland_matrix) const;
		std::vector<bool> smith_set(
			const copeland_scenario & scenario) const;
		size_t smith_set_size(const std::vector<bool> & smith) const;
		size_t smith_set_size(const copeland_scenario & scenario) const;

		constraint_set interposing_constraints;
		copeland_scenario inner_before, inner_after;

		bool set_scenario_constraints(copeland_scenario before,
			copeland_scenario after, const std::string before_name, 
			const std::string after_name);
};