#include "scored_method.h"

class birational_eval : public scored_method {
	private:
		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot);

	public:
		std::string name() const {
			return "Cardinal: Birational";
		}

		bool maximize() const {
			return true;
		}
};

double birational_eval::evaluate(combo::it & start, combo::it & end,
	const scored_ballot & this_ballot) {

	//                                     x_w
	// L(w) = SUM      SUM      SUM      -------
	// 	  vote     w in W   s in W   1 + x_s
	// 	  vectors
	// 	  x->

	// From https://rangevoting.org/WarrenSmithPages/homepage/multisurv.pdf
	// method 7.8, "the birational system".

	// We don't handle Range-style "no opinion" ballots yet - they
	// get set to 0. I might just snip that out of scored_method since
	// I'm not using it :-P

	// I thought that replacing the one in the denominator with a C
	// could generalize to more or less proportionality vs. single-winner
	// quality, but that turned out to be wrong; using 0.5 instead just
	// degrades the proportionality without any compensation.

	double total = 0;

	// Minimax doesn't do much better either.

	for (auto pos = start; pos != end; ++pos) {
		size_t w = *pos;

		for (auto sec_pos = start; sec_pos != end; ++sec_pos) {
			size_t s = *sec_pos;

			double atw = this_ballot.scores[w],
				   ats = this_ballot.scores[s];

			if (!isfinite(atw)) {
				atw = 0;
			}
			if (!isfinite(ats)) {
				ats = 0;
			}

			total += atw / (1 + ats);
		}
	}

	return total * this_ballot.weight;
}

typedef exhaustive_method_runner<birational_eval> birational;
