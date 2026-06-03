#pragma once

#include "scored_method.h"
#include "tools/tools.h"

#include "multiwinner/helper/digamma.h"

class psi_voting_eval : public scored_method {
	private:
		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot);

		double delta;

	public:
		std::string name() const {
			if (delta == 0.5) {
				return "Cardinal: Psi (Sainte-Laguë)";
			}
			if (delta == 1) {
				return "Cardinal: Psi (D'Hondt)";
			}
			return "Cardinal: Psi (delta = " + dtos(delta) + ")";
		}

		bool maximize() const {
			return true;
		}

		psi_voting_eval() {
			delta = 0.5;
		}

		psi_voting_eval(double delta_in) {
			if (delta_in < 0) {
				throw std::invalid_argument("Psi voting: delta can't be negative");
			}

			delta = delta_in;
		}
};

inline double psi_voting_eval::evaluate(combo::it & start, combo::it & end,
	const scored_ballot & this_ballot) {

	// Sum over voters v: digamma( delta +
	//		sum over elected candidates c: v's score of c)

	// From https://rangevoting.org/QualityMulti.html

	double norm_rating_sum = 0;

	for (auto pos = start; pos != end; ++pos) {
		norm_rating_sum += this_ballot.get_norm_score(*pos);
	}

	// digamma(0) = infinity. HACK to deal with this without having
	// to bring in infinities.
	if (delta + norm_rating_sum == 0) {
		return 1e9 * this_ballot.weight;
	}

	return digamma(delta + norm_rating_sum) * this_ballot.weight;
}

typedef exhaustive_method_runner<psi_voting_eval> psi_voting;
