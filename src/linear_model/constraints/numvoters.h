#pragma once

#include <string>

#include "../lin_relation/constraint_set.h"

// Generate constraints dealing with the number of voters.

class voter_constraints {
	private:
		std::string voters_var;

	public:
		constraint_set max_numvoters_definition(int numcands,
			std::string situation_suffix) const;

		constraint_set max_numvoters_upper_bound(int maximum) const;

		std::string get_voters_var() {
			return voters_var;
		}

		voter_constraints(std::string voters_var_name_in) {
			voters_var = voters_var_name_in;
		}
};