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
		bool maximize, opt_direction_set;

		double minimum, maximum;
		std::vector<std::vector<size_t> >
		minimum_solutions, maximum_solutions;

	public:

		// should be set by the combinatorial method
		exhaustive_optima(bool maximize_in) {
			maximize = maximize_in;
			opt_direction_set = true;

			minimum = std::numeric_limits<double>::signaling_NaN();
			maximum = std::numeric_limits<double>::signaling_NaN();
		}

		exhaustive_optima() {
			// We have to set a maximization direction as otherwise
			// it will count as an undefined value and may trip compiler
			// warnings. However, we never use this direction since
			// opt_direction_set is false.
			maximize = false;
			opt_direction_set = false;

			minimum = std::numeric_limits<double>::signaling_NaN();
			maximum = std::numeric_limits<double>::signaling_NaN();
		}

		// Set whether to maximize or minimize.
		void set_optimum_direction(bool maximize_in) {
			maximize = maximize_in;
			opt_direction_set = true;
		}

		void update(double objective_value,
			std::vector<size_t>::const_iterator & solution_start,
			std::vector<size_t>::const_iterator & solution_end) {
			// a NaN indicates a solution that's inadmissible or
			// violates constraints.
			if (isnan(objective_value)) {
				return;
			}

			if (isnan(minimum) || objective_value <= minimum) {
				if (objective_value < minimum) {
					minimum_solutions.clear();
				}

				minimum = objective_value;
				minimum_solutions.push_back(std::vector<size_t>(
						solution_start, solution_end));
			}
			if (isnan(maximum) || objective_value >= maximum) {
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
				throw std::runtime_error("Exhaustive optima: Trying to get optimum"
					" without having evaluated anything.");
			}

			// TODO? do something random here? Kinda hard to see
			// what would be the right design option; doing random
			// might be clone-dependent in all sorts of nasty ways.
			// Picking the first might violate neutrality.
			return maximum_solutions[0];
		}

		std::vector<size_t> get_minimum_solution() const {
			if (maximum_solutions.empty()) {
				throw std::runtime_error("Exhaustive optima: Trying to get optimum"
					" without having evaluated anything.");
			}

			return minimum_solutions[0];
		}

		std::vector<size_t> get_optimal_solution() const {
			if (!opt_direction_set) {
				throw std::runtime_error("Exhaustive optima: Init bug! No direction"
					" of optimization has been set.");
			}
			if (maximize) {
				return get_maximum_solution();
			} else {
				return get_minimum_solution();
			}
		}

		std::vector<std::vector<size_t> > get_maximum_solutions() const {
			if (maximum_solutions.empty()) {
				throw std::runtime_error("Exhaustive optima: Trying to get optimum"
					" without having evaluated anything.");
			}

			return maximum_solutions;
		}

		std::vector<std::vector<size_t> > get_minimum_solutions() const {
			if (maximum_solutions.empty()) {
				throw std::runtime_error("Exhaustive optima: Trying to get optimum"
					" without having evaluated anything.");
			}

			return minimum_solutions;
		}

		std::vector<std::vector<size_t> > get_optimal_solutions() const {
			if (!opt_direction_set) {
				throw std::runtime_error("Exhaustive optima: Init bug! No direction"
					" of optimization has been set.");
			}
			if (maximize) {
				return get_maximum_solutions();
			} else {
				return get_minimum_solutions();
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
			assert(opt_direction_set);
			if (maximize) {
				return get_maximum();
			} else {
				return get_minimum();
			}
		}
};
