#pragma once

#include "score_aggregate.h"

#include <limits>
#include <stdexcept>
#include <math.h>

class sum_scores : public score_aggregate {
	private:
		// The value should never be exposed, but it could technically
		// happen if asserts are off and we do comparisons, so set it
		// to NaN.
		double value = std::numeric_limits<double>::signaling_NaN();
		bool value_set = false;

		// We handle infinities as constants M in the limit of M->inf.
		// This means that whichever side has more +infinities and fewer
		// -infinities is greater than the other; if their net infinity
		// weight (weight of +inf - weight of -inf) is equal, then the
		// finite values decide.

		double net_infinity_weight = 0;

		ordinal three_way_compare(const sum_scores & other) const {
			if (net_infinity_weight > other.net_infinity_weight) {
				return GREATER;
			}
			if (net_infinity_weight < other.net_infinity_weight) {
				return LESS;
			}

			assert(value_set && other.value_set); // otherwise this is invalid

			if (value > other.value) {
				return GREATER;
			}
			if (value < other.value) {
				return LESS;
			}

			return EQUAL;
		}

	public:
		bool operator<(const sum_scores & other) const {
			return three_way_compare(other) == LESS;
		}

		bool operator<=(const sum_scores & other) const {
			return three_way_compare(other) != GREATER;
		}

		bool operator>(const sum_scores & other) const {
			return three_way_compare(other) == GREATER;
		}

		bool operator>=(const sum_scores & other) const {
			return three_way_compare(other) != LESS;
		}

		bool operator==(const sum_scores & other) const {
			return three_way_compare(other) == EQUAL;
		}

		bool operator!=(const sum_scores & other) const {
			return three_way_compare(other) != EQUAL;
		}

		void insert(double weight, double score) {
			if (weight <= 0 || isnan(weight) || !isfinite(weight)) {
				throw std::invalid_argument(
					"sum_scores: tried to insert with invalid weight (non-positive or non-finite)");
			}

			if (isnan(score)) {
				throw std::invalid_argument(
					"sum_scores: tried to insert NaN score");
			}

			// Handle infinities
			if (!isfinite(score)) {
				if (score > 0) {
					net_infinity_weight += weight;
				} else {
					net_infinity_weight -= weight;
				}
				return;
			}

			if (value_set) {
				value += weight * score;
			} else {
				value = weight * score;
				value_set = true;
			}
		}
};
