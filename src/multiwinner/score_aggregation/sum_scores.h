#include "score_aggregate.h"

#include <stdexcept>

class sum_scores : public score_aggregate {
	private:
		double value;
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
			if (weight < 0 || isnan(weight) || isnan(score)) {
				throw std::invalid_argument(
					"sum_scores: negative weight or NaN score");
			}

			if (value_set) {
				value += weight * score;
			} else {
				value = weight * score;
				value_set = true;
			}
		}
};