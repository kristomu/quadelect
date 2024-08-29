#include "scored_method.h"
#include "tools/tools.h"

class log_penalty_eval : public scored_method {
	private:
		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot);

		double K;

	public:
		std::string name() const {
			if (K == 0) {
				return "Cardinal: LPV0+";
			} else {
				return "Cardinal: Log-penalty (K = " + dtos(K) + ")";
			}
		}

		bool maximize() const {
			return false;
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

double log_penalty_eval::evaluate(combo::it & start, combo::it & end,
	const scored_ballot & this_ballot) {

	/*
	                                        /     K + |W|    \
	   L_k(W) = SUM     SUM       x_j *   ln| -------------- |
	            vote    j in C              | K + SUM    x_s |
	            vector                      \     s in W     /
	*/

	// From https://rangevoting.org/WarrenSmithPages/homepage/multisurv.pdf
	// method 7.9, "Logarithmic penalty voting".

	// We might get infinities if K=0. In that case, we should score
	// by the number of infinities and then secondarily on the sum of
	// the finite values. Done with somewhat of a hack so far; I'm going
	// to need another type if it's to be done properly, and threading
	// that through the base class is going to be annoying.

	// ----- //

	// First calculate the inner (logarithm) term, since it only
	// depends on the winners.

	// Calculate the denominator.
	double inner_denominator = 0;

	for (auto pos = start; pos != end; ++pos) {
		inner_denominator += this_ballot.scores[*pos];
	}

	if (inner_denominator == 0 && K == 0) {
		// This will return an infinity. Return a very large value as
		// a HACK.
		return this_ballot.weight * 1e15;
	}

	// Otherwise calculate the product.

	double voter_contribution = 0;
	size_t num_seats = end - start;
	double log_denom = log((K +  num_seats) / (K + inner_denominator));

	for (auto score_pos = this_ballot.scores.begin();
		score_pos != this_ballot.scores.end(); ++score_pos) {
		voter_contribution += *score_pos * log_denom;
	}

	return voter_contribution * this_ballot.weight;
}

typedef exhaustive_method_runner<log_penalty_eval> log_penalty;