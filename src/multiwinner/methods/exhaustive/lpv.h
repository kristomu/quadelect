#pragma once

// TODO: Implement test from figure 8.1. (and possibly also 8.3) of
// https://rangevoting.org/WarrenSmithPages/homepage/multisurv.pdf
// Scale the ratings to some large multiple so that renorm is tested.

// 8.6 might make a good LPV0+-specific test.

#include "scored_method.h"
#include "tools/tools.h"

class log_penalty_eval : public scored_method {
	private:
		void add_method_score(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot,
			score_aggregate & output);

		double K;

	public:
		std::string name() const {
			if (K == 0) {
				return "Cardinal: LPV0+";
			} else {
				return "Cardinal: Log-penalty (K = " + dtos(K) + ")";
			}
		}

		log_penalty_eval() {
			K = 0;
		}

		log_penalty_eval(double K_in) {
			if (K_in < 0) {
				throw std::invalid_argument("log penalty: K can't be negative.");
			}

			K = K_in;
		}
};

inline void log_penalty_eval::add_method_score(combo::it & start,
	combo::it & end, const scored_ballot & this_ballot,
	score_aggregate & output) {

	/*
	                                        /     K + |W|    \
	   L_k(W) = SUM     SUM       x_j *   ln| -------------- |
	            vote    j in C              | K + SUM    x_s |
	            vector                      \     s in W     /
	             x

	   to be minimized, with x_s being voter x's rating of candidate s,
	   C being the set of all candidates, and W the set of winners.
	*/

	// From https://rangevoting.org/WarrenSmithPages/homepage/multisurv.pdf
	// method 7.9, "Logarithmic penalty voting".

	// We might get infinities if K=0, but that is not a problem:
	// the score aggregate classes can manage.

	// ----- //

	// First calculate the inner (logarithm) term, since it only
	// depends on the winners.

	// Calculate the denominator.
	double inner_denominator = 0;

	for (auto pos = start; pos != end; ++pos) {
		inner_denominator += this_ballot.get_norm_score(*pos);
	}

	double penalty = 0;

	// If we have an infinity, add it directly.
	if (inner_denominator == 0 && K == 0) {
		penalty = std::numeric_limits<double>::infinity();
	} else {
		// Otherwise calculate the product.

		double voter_contribution = 0;
		size_t num_seats = end - start;
		double log_denom = log((K +  num_seats) / (K + inner_denominator));

		for (size_t candidate = 0; candidate < this_ballot.scores.size();
			++candidate) {
			voter_contribution += log_denom *
				this_ballot.get_norm_score(candidate);
		}

		penalty = voter_contribution;
	}

	output.insert(this_ballot.weight, -penalty);
}

typedef exhaustive_method_runner<log_penalty_eval> log_penalty;
