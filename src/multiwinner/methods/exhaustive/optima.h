// This is a class that holds the current maxima and minima found
// for an exhaustive multiwinner method. It also contains the actual
// solutions (councils).

// We do it like this due to peculiarities in the external
// for_each_permutation implementation.

// Since the exhaustive_optima class is mostly just getters and setters,
// I've put it all in the header.

// Simplified to only hold maxima now; if your method uses penalties
// (more is worse), negate them before using them. This should make
// egalitarian/minmax method handling much easier and free up a bunch
// of otherwise duplicate code.

#pragma once

#include <vector>
#include <numeric>

#include "multiwinner/score_aggregation/sum_scores.h"

class exhaustive_optima {
	private:
		bool is_set = false;
		sum_scores maximum;
		std::vector<std::vector<size_t> > maximum_solutions;

	public:
		void update(
			std::vector<size_t>::const_iterator solution_start,
			std::vector<size_t>::const_iterator solution_end,
			sum_scores objective_value) {

			// If the maximum isn't set, just set it directly.
			// We need to do this because the comparison operator
			// result might not be well-defined if the maximum hasn't
			// ben set.

			if (!is_set) {
				is_set = true;
				maximum = objective_value;
				maximum_solutions.push_back(std::vector<size_t>(
						solution_start, solution_end));

				return;
			}

			if (objective_value >= maximum) {
				if (objective_value > maximum) {
					maximum_solutions.clear();
				}

				maximum = objective_value;
				maximum_solutions.push_back(std::vector<size_t>(
						solution_start, solution_end));
			}
		}

		std::vector<size_t> get_maximum_solution() const {
			if (maximum_solutions.empty()) {
				throw std::runtime_error("Exhaustive optima: Trying to get maximum"
					" without having evaluated anything.");
			}

			// TODO? do something random here? Kinda hard to see
			// what would be the right design option; doing random
			// might be clone-dependent in all sorts of nasty ways.
			// Picking the first might violate neutrality.
			return maximum_solutions[0];
		}

		std::vector<std::vector<size_t> > get_maximum_solutions() const {
			if (maximum_solutions.empty()) {
				throw std::runtime_error("Exhaustive optima: Trying to get maximum"
					" without having evaluated anything.");
			}

			return maximum_solutions;
		}

		sum_scores get_maximum() const {
			if (maximum_solutions.empty()) {
				throw std::runtime_error("Exhaustive optima: Trying to get maximum"
					" without having evaluated anything.");
			}
			return maximum;
		}
};
