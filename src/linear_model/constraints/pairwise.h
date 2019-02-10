#pragma once

#include <string>

#include "../lin_relation/lin_relation.h"
#include "../lin_relation/constraint_set.h"

// Class for constructing constraints that limits a particular permutation
// set to a particular Copeland scenario (e.g. ABCA cycle, or more complex
// four-candidate cycles, or where A is the CW).

class pairwise_constraints {
	private:
		std::string margin;

		lin_relation get_beat_equation(bool a_beats_b, int a, int b,
			std::string suffix, int numcands) const;

	public:
		constraint_set beat_constraints(
			const std::vector<bool> & short_form_copeland, 
			std::string suffix, int numcands) const;

		pairwise_constraints(std::string margin_name_in) {
			margin = margin_name_in;
		}

		pairwise_constraints() {
			margin = "min_victory_margin";
		}
};