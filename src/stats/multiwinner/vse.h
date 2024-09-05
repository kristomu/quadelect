#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>

// VSE container.

// This works both for scores and penalties; it doesn't matter if higher values
// are better or worse, since the signs cancel out in the VSE calculation.

class VSE {
	private:
		double total_random = 0; // Sum of (expected) scores for a random choice
		double total_best = 0;   // Sum of optimal scores
		double total_method = 0; // Sum of scores given to the method.

		double last_random, last_best, last_method;

		size_t rounds = 0;

	public:
		void add_result(double random_result, double method_result,
			double best_result) {

			total_random += random_result;
			total_method += method_result;
			total_best += best_result;

			last_random = random_result;
			last_method = method_result;
			last_best = best_result;

			++rounds;
		}

		double get() const {
			return (total_method - total_random) /
				(total_best - total_random);
		}

		double get_this_round() const {
			return (last_method - last_random) /
				(last_best - last_random);
		}

		double get_this_round_random() const {
			return last_random;
		}

		double get_this_round_best() const {
			return last_best;
		}

		double get_this_round_raw() const {
			return last_method;
		}

		// Adds the last result data from another VSE object.
		void add_last(const VSE & other) {
			if (other.get_rounds() == 0) {
				throw std::invalid_argument("VSE add_last given "
					"VSE object with no data!");
			}

			add_result(other.get_this_round_random(),
				other.get_this_round_raw(),
				other.get_this_round_best());
		}

		size_t get_rounds() const {
			return rounds;
		}
};

// For storing multiple VSEs together, e.g. proportionality and utilitarian
// efficiency.

typedef std::vector<VSE> VSE_point;