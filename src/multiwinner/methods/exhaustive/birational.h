#pragma once
#include "scored_method.h"

class birational_eval : public scored_method {
	private:
		void add_method_score(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot,
			score_aggregate & output);

		double K;
		bool relative;

	public:
		std::string name() const {

			// For "absolute" as in absolute scale
			std::string rel_abs = "A-Birational";

			if (relative) {
				rel_abs = "R-Birational"; // [R]elative, i.e. normalized.
			}

			if (K == 1) {
				return "Cardinal: " + rel_abs;
			} else {
				return "Cardinal: " + rel_abs + " ext (" + dtos(K) + ")";
			}
		}

		birational_eval() {
			K = 1;
			relative = true;
		}

		birational_eval(double K_in) {
			K = K_in;
			relative = true;
		}

		birational_eval(double K_in, bool relative_in) {
			K = K_in;
			relative = relative_in;
		}

};

inline void birational_eval::add_method_score(
	combo::it & start, combo::it & end,
	const scored_ballot & this_ballot, score_aggregate & output) {

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
	// (TODO: Check that now that we've got proportionality vs VSE going)

	// It might also be a good idea to try normalization here as well,
	// particularly given the extreme results it's producing...

	double total = 0, atw, ats;

	for (auto pos = start; pos != end; ++pos) {
		size_t w = *pos;

		for (auto sec_pos = start; sec_pos != end; ++sec_pos) {
			size_t s = *sec_pos;

			if (relative) {
				atw = this_ballot.get_norm_score(w);
				ats = this_ballot.get_norm_score(s);
			} else {
				atw = this_ballot.scores[w];
				ats = this_ballot.scores[s];
			}

			if (!isfinite(atw)) {
				atw = 0;
			}
			if (!isfinite(ats)) {
				ats = 0;
			}

			total += atw / (K + ats);
		}
	}

	output.insert(this_ballot.weight, total);
}

typedef exhaustive_method_runner<birational_eval> birational;
