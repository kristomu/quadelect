#pragma once

// This is not quite a relative criterion, because it depends on
// scenarios, not just on the number of candidates. Thus it has to be
// implemented here rather than in linear_model.

#include "vector_ballot.h"

#include "../../../../linear_model/constraints/numvoters.h"
#include "../../../../linear_model/constraints/pairwise.h"
#include "../../../../linear_model/constraints/general.h"

#include <vector>

struct scenario_reduction {
	std::vector<int> cand_relabeling;
	copeland_scenario to_scenario;
};

class isda_reduction {
	private:
		scenario_reduction get_ISDA_reduction(
			const copeland_scenario & in) const;

	public:
		std::vector<bool> smith_set(const std::vector<std::vector<bool> > &
			copeland_matrix) const;
		std::vector<bool> smith_set(
			const copeland_scenario & scenario) const;
		size_t smith_set_size(const std::vector<bool> & smith) const;
		size_t smith_set_size(const copeland_scenario & scenario) const;

		constraint_set interposing_constraints;
		copeland_scenario inner_before, inner_after;
		std::vector<int> elimination_spec;
		bool alters_before, alters_after;

		bool set_scenario_constraints(copeland_scenario before,
			copeland_scenario after, const std::string before_name,
			const std::string after_name);

		bool is_interposing_required() const {
			return !interposing_constraints.empty();
		}
};