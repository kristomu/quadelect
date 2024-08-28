#include "scored_method.h"

class birational : public scored_method {
	private:
		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot) const;

	public:
		std::string name() const {
			return "Birational";
		}
}

double birational::evaluate(combo::it & start, combo::it & end,
	const scored_ballot & this_ballot) const {

	//                                     x_w
	// L(w) = SUM      SUM      SUM      -------
	// 	  vote     w in W   s in W       1 + x_s
	// 	  vectors
	// 	  x->

	// We don't handle Range-style ballots yet - they get set to 0.
	// Possible later TODO, change 1 + x_s so that the D'Hondt
	// generalization of PAV turns into Sainte-Laguë instead.

	double total = 0;

	// Minimax doesn't do much better either.

	for (auto pos = start; pos != end; ++pos) {
		size_t w = *pos;

		if (!W[w]) {
			continue;
		}

		for (auto sec_pos = start; sec_pos != end; ++sec_pos) {
			size_t s = *sec_pos;

			if (!W[s]) {
				continue;
			}
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