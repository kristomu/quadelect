#pragma once

#include "scored_method.h"
#include "tools/tools.h"

#include "multiwinner/helper/digamma.h"

// TESTED: This agrees with Warren Smith's CleanOptPRVote.c,
// https://www.rangevoting.org/CleanOptPRVote.c

// as long as every ballot both rates someone minimum and
// someone maximum. Otherwise, Quadelect's renormalization
// uses more of the span than CleanOptPRVote does, which can
// lead to discrepancies.

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
	// where scores/ratings are normalized so that the max one may cast
	// is 1, and minimum is 0.

	// From https://rangevoting.org/QualityMulti.html

	double norm_rating_sum = 0;

	for (auto pos = start; pos != end; ++pos) {
		norm_rating_sum += this_ballot.get_norm_score(*pos);
	}

	// digamma(0) = +/- infinity. HACK to deal with this without having
	// to bring in infinities.
	// Since we're given normalized scores, these can't ever be below
	// zero, so it makes more sense to take the limit approaching from
	// the positive side, i.e. x -> 0+; and intuitively, it makes sense
	// that not being represented (all scores 0) is bad, not good.
	if (delta + norm_rating_sum == 0) {
		return -1e9 * this_ballot.weight;
	}

	return digamma(delta + norm_rating_sum) * this_ballot.weight;
}

typedef exhaustive_method_runner<psi_voting_eval> psi_voting;
typedef sequential_method_runner<psi_voting_eval> sequential_psi_voting;