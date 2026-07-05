#include "score_aggregate.h"

#include <stdexcept>
#include <math.h>

class sum_scores : public score_aggregate {
	private:
		// The value should never be exposed, but it could technically
		// happen if asserts are off and we do comparisons, so set it
		// to NaN.
		double value = std::numeric_limits<double>::signaling_NaN();
		bool value_set = false;

	public:
		bool operator<(const sum_scores & other) const {
			assert(value_set && other.value_set); // otherwise this is invalid
			return value < other.value;
		}

		bool operator<=(const sum_scores & other) const {
			assert(value_set && other.value_set);
			return value <= other.value;
		}

		bool operator>(const sum_scores & other) const {
			assert(value_set && other.value_set);
			return value > other.value;
		}

		bool operator>=(const sum_scores & other) const {
			assert(value_set && other.value_set);
			return value >= other.value;
		}

		bool operator==(const sum_scores & other) const {
			assert(value_set && other.value_set);
			return value == other.value;
		}

		bool operator!=(const sum_scores & other) const {
			assert(value_set && other.value_set);
			return value != other.value;
		}

		void insert(double weight, double score) {
			if (weight <= 0 || isnan(weight)) {
				throw std::invalid_argument(
					"sum_scores: tried to insert with invalid weight (non-positive or NaN)");
			}

			if (isnan(score)) {
				throw std::invalid_argument(
					"sum_scores: tried to insert NaN score");
			}

			if (value_set) {
				value += weight * score;
			} else {
				value = weight * score;
				value_set = true;
			}
		}
};