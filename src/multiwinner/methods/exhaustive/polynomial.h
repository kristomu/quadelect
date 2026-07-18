#pragma once

#include "scored_method.h"
#include "tools/tools.h"

#include "multiwinner/helper/digamma.h"

// This is a Harmonic-like method based on Warren's "linear" structure:
// Q(voter v) =
// 	f(sum i=1...s (ith highest rating by v for someone in outcome) * A_i)
// where s is the number of seats, f is some function, and A_1,..., are
// constants.

// It turns out that if we decide on a particular f, then the "proper" values
// for A_i can be determined by inverting f, if we desire the quality function
// Q to work like PAV's when everybody votes approval-style.

// Given this observation, we can let f be a polynomial and experiment with
// what kind of slope between the fixed Approval-style points maximizes some
// quality of PR.

// with f = x^p for a tunable p, we have
// 	A_1 = 1
// 	A_k = (sum i=1..k 1/i)^-p - sum i=1..(k-1) A_i
// for a d'Hondt-style method. The solutions for other values of
// delta are analogous.

class polynomial_eval : public scored_method {
	private:
		void add_method_score(const combo::it & start,
			const combo::it & end,
			const scored_ballot & this_ballot,
			score_aggregate & output);

		double p, delta;

		std::vector<double> elected_ratings;

	public:
		std::string name() const {
			return "Cardinal: Polynomial (p = " + dtos(p) +
				", delta = " + dtos(delta) + ")";
		}

		polynomial_eval() {
			p = 1;
			delta = 0.5; // Sainte-Laguë
		}

		polynomial_eval(double p_in) {
			p = p_in;
			delta = 0.5; // ditto
		}

		polynomial_eval(double p_in, double delta_in) {
			p = p_in;
			delta = delta_in;
		}
};

inline void polynomial_eval::add_method_score(
	const combo::it & start, const combo::it & end,
	const scored_ballot & this_ballot, score_aggregate & output) {

	size_t council_size = end - start;

	// Calculate A_i values, kinda hacky and redundant
	// This could be done once instead of once every time.

	// Note that the generalized_harmonic values are unnormalized
	// like in the Harmonic voting implementation - e.g. for Sainte-
	// Laguë, the first weight factor is 1/0.5 = 2. If you want to
	// normalize so that the first factor is always unity, then multiply
	// the generalized_harmonic below by delta. It should make no
	// difference to the outcome.

	std::vector<double> A(council_size);

	double A_cumulative = 0;

	for (size_t i = 0; i < A.size(); ++i) {
		double generalized_harmonic;
		if (delta == 0) {
			// limit of the digamma expression * delta
			// as delta -> 0.
			generalized_harmonic = 1;
		} else {
			generalized_harmonic =
				(digamma(i+delta+1) - digamma(delta));
		}

		A[i] = pow(generalized_harmonic, 1.0/p) - A_cumulative;
		A_cumulative += A[i];
	}

	elected_ratings.resize(council_size);

	double quality = 0;
	size_t idx = 0;

	for (auto pos = start; pos != end; ++pos) {
		elected_ratings[idx++] = this_ballot.get_norm_score(*pos);
	}

	std::sort(elected_ratings.begin(), elected_ratings.begin()+idx,
		std::greater<double>());

	// Note that this will work as a "no opinion" marker; if the voter
	// hasn't rated anybody, then we don't return below-minimum, we
	// just don't update anything.
	// This might lead to strange consequences with egalitarian mode;
	// check later (compare to adding zero, etc).
	if (elected_ratings.empty()) {
		return;
	}

	// Like Harmonic, delta = 0 focuses all the voter's weight on
	// the voter's favorite.

	if (delta == 0) {
		output.insert(this_ballot.weight, elected_ratings[0]);
		return;
	}

	for (size_t i = 0; i < council_size; ++i) {
		quality += elected_ratings[i] * A[i];
	}

	output.insert(this_ballot.weight, pow(quality, p));
}
