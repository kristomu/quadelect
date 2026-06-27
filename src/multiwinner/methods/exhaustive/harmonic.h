#pragma once

#include "scored_method.h"
#include "tools/tools.h"

// Harmonic voting reduces to Chamberlin-Courant in the limit of delta
// approaching zero, because Chamberlin-Courant consists of assigning
// candidates to voters so as to maximize the total ratings of voters
// for their candidates. As we approach the limit, the division by
// values increasingly closer to zero blows up the contribution of the
// rating of each voter's favorite candidate among those elected, which
// leads to the given reduction.

// See e.g. https://www.ijcai.org/proceedings/2022/0069.pdf

class harmonic_voting_eval : public scored_method {
	private:
		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot);

		double delta;

		std::vector<double> elected_ratings;

	public:
		std::string name() const {
			if (delta == 0) {
				return "Cardinal: Harmonic (Chamberlin-Courant)";
			}
			if (delta == 0.5) {
				return "Cardinal: Harmonic (Sainte-Laguë)";
			}
			if (delta == 1) {
				return "Cardinal: Harmonic (D'Hondt)";
			}
			// TODO? a better scale where e.g. 2 is pure Range (delta=infty)
			// and 1 is the same, and the spread (angle from 0 in polar coords)
			// is approximately linear in this value.
			if (delta > 1) {
				return "Cardinal: Harmonic extended (" + dtos(delta) + ")";
			}
			return "Cardinal: Harmonic (delta = " + dtos(delta) + ")";
		}

		bool maximize() const {
			return true;
		}

		harmonic_voting_eval() {
			delta = 0.5;
		}

		harmonic_voting_eval(double delta_in) {
			if (delta_in < 0) {
				throw std::invalid_argument("Harmonic voting: "
					"delta can't be negative");
			}

			delta = delta_in;
		}
};


inline double harmonic_voting_eval::evaluate(combo::it & start,
	combo::it & end, const scored_ballot & this_ballot) {

	// Sort the ratings given to the candidates proposed for election
	// by this voter in descending order. Then the quality contributed
	// to the proposed seat assignment by the voter is

	// sum i=1..|S| : (ith greatest rating of candidate
	//	for someone to be elected)/ (i-1 + delta).

	// From https://rangevoting.org/QualityMulti.html

	size_t council_size = end - start;

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
	// return zero.
	if (elected_ratings.empty()) {
		return 0;
	}

	// Handle division by zero (delta = 0) by only caring about the
	// voter's favorite elected candidate. (See comment near the start
	// of the file for how this makes sense.)

	if (delta == 0) {
		return this_ballot.weight * elected_ratings[0];
	}

	// Otherwise, do full Harmonic.

	for (size_t i = 0; i < council_size; ++i) {
		quality += elected_ratings[i] / (i + delta);
	}

	return this_ballot.weight * quality;
}

typedef exhaustive_method_runner<harmonic_voting_eval> harmonic_voting;
typedef sequential_method_runner<harmonic_voting_eval>
sequential_harmonic_voting;
