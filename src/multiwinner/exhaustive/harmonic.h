#include "scored_method.h"
#include "tools/tools.h"

class harmonic_voting_eval : public scored_method {
	private:
		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot);

		double delta;

		std::vector<double> elected_ratings;

	public:
		std::string name() const {
			if (delta == 0.5) {
				return "Cardinal: Harmonic (Sainte-Laguë)";
			}
			if (delta == 1) {
				return "Cardinal: Harmonic (D'Hondt)";
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


double harmonic_voting_eval::evaluate(combo::it & start, combo::it & end,
	const scored_ballot & this_ballot) {

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

	for (size_t i = 0; i < council_size; ++i) {
		quality += elected_ratings[i] / (i + delta);
	}

	return quality;
}

typedef exhaustive_method_runner<harmonic_voting_eval> harmonic_voting;