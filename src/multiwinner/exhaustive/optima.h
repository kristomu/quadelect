// This is a class that holds the current maxima and minima found
// for an exhaustive multiwinner method. It also contains the actual
// solutions (councils).

// We do it like this due to peculiarities in the external
// for_each_permutation implementation.

// Since the exhaustive_optima class is mostly just getters and setters,
// I've put it all in the header.

#pragma once

#include <vector>
#include <numeric>

class exhaustive_optima {
	private:
		bool maximize;

		double minimum, maximum;
		std::vector<size_t> minimum_solution, maximum_solution;

	public:

		// should be set by the combinatorial method
		exhaustive_optima(bool maximize_in) {
			maximize = maximize_in;

			minimum = std::numeric_limits<double>::signaling_NaN();
			maximum = std::numeric_limits<double>::signaling_NaN();
		}

		void update(double objective_value,
			std::vector<size_t>::const_iterator & solution_start,
			std::vector<size_t>::const_iterator & solution_end) {
			// a NaN indicates a solution that's inadmissible or
			// violates constraints.
			if (isnan(objective_value)) {
				return;
			}

			if (isnan(minimum) || objective_value < minimum) {
				minimum = objective_value;
				minimum_solution = std::vector<size_t>(
						solution_start, solution_end);
			}
			if (isnan(maximum) || objective_value > maximum) {
				maximum = objective_value;
				maximum_solution = std::vector<size_t>(
						solution_start, solution_end);
			}
		}

		std::vector<size_t> get_maximum_solution() const {
			return maximum_solution;
		}

		std::vector<size_t> get_minimum_solution() const {
			return minimum_solution;
		}

		std::vector<size_t> get_optimal_solution() const {
			if (maximize) {
				return get_maximum_solution();
			} else {
				return get_minimum_solution();
			}
		}

		double get_maximum() const {
			assert(!isnan(maximum));
			return maximum;
		}

		double get_minimum() const {
			assert(!isnan(minimum));
			return minimum;
		}

		double get_optimum() const {
			if (maximize) {
				return get_maximum();
			} else {
				return get_minimum();
			}
		}
};