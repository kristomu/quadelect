#include "scored_method.h"
#include "tools/tools.h"

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

// Lightly adapted from
// https://github.com/lteacy/decBRL-cpp/blob/master/src/polygamma/digamma.cpp

double digamma(double x) {
	double c = 8.5;
	double d1 = -0.5772156649;
	double r;
	double s = 0.00001;
	double s3 = 0.08333333333;
	double s4 = 0.0083333333333;
	double s5 = 0.003968253968;
	double value;
	double y;

	//  Check the input.

	if (x <= 0.0) {
		throw std::invalid_argument("digamma: x < 0 is not supported");
	}

	//  Initialize.

	y = x;
	value = 0.0;

	//  Use approximation if argument <= S.

	if (y <= s) {
		value = d1 - 1.0 / y;
		return value;
	}

	//  Reduce to DIGAMA(X + N) where (X + N) >= C.

	while (y < c) {
		value = value - 1.0 / y;
		y = y + 1.0;
	}

	//  Use Stirling's (actually de Moivre's) expansion if argument > C.

	r = 1.0 / y;
	value = value + log(y) - 0.5 * r;
	r = r * r;
	value = value - r * (s3 - r * (s4 - r * s5));

	return value;
}

double psi_voting_eval::evaluate(combo::it & start, combo::it & end,
	const scored_ballot & this_ballot) {

	// Sum over voters v: digamma( delta +
	//		sum over elected candidates c: v's score of c)

	// From https://rangevoting.org/QualityMulti.html

	double norm_rating_sum = 0;

	for (auto pos = start; pos != end; ++pos) {
		norm_rating_sum += this_ballot.get_norm_score(*pos);
	}

	return digamma(delta + norm_rating_sum) * this_ballot.weight;
}

typedef exhaustive_method_runner<psi_voting_eval> psi_voting;