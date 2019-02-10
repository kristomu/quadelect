#pragma once
#include "../lin_relation/constraint_set.h"
#include "../lin_relation/constraint.h"

#include <vector>

// General-purpose constraints

class general_const {

	public:
		static constraint_set all_nonnegative(const constraint_set &
			relevant_constraints) {

			std::vector<constraint> nonnegs;

			for (std::string var : relevant_constraints.get_free_variables()) {
				constraint nonneg;

				nonneg.description = "nonneg_" + var;
				nonneg.constraint_rel.type = LREL_GE;
				nonneg.constraint_rel.lhs.weights.push_back(
					std::pair<std::string, double>(var, 1));
				nonneg.constraint_rel.rhs.constant = 0; // var >= 0

				nonnegs.push_back(nonneg);
			}

			return nonnegs;
		}
};