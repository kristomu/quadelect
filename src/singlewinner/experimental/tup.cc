// The "tup" method and a similar Tup/SV method.
// If there are more than three candidates, abort (undefined). Otherwise,
// A's score is B>C (If A beats B) + C>B (if A beats C).
// Same for the other candidates.
// Tup/SV: A's score is B>C/(epsilon+C>A) instead. Ideally, take the rank as 
// epsilon->0.

#include "tup.h"

pair<ordering, bool> tup::pair_elect(const abstract_condmat & input,
		const vector<bool> & hopefuls, cache_map * cache, 
		bool winner_only) const {

	ordering out;

	assert (input.get_num_candidates() == 3);

	for (int counter = 0; counter < 3; ++counter) {
		double score = 0;

		// Cycle A>B>C>A
		// then counter = 0 (A)
		// sec = 1 (B),
		// third party = 2 (C)

		// SV would presumably be:
		// (B>C) / (C>A)

		for (int sec = 0; sec < 3; ++sec) {
			if (counter == sec) continue;

			int third_party = -1;
			for (int tet = 0; tet < 3 && third_party == -1; ++tet)
				if (counter != tet && sec != tet)
					third_party = tet;
			// Now suppose we're A and the cycle is A>B>C>A.
			double ab = input.get_magnitude(counter, sec);
			double bc = input.get_magnitude(sec, third_party);
			double ca = input.get_magnitude(third_party, counter);
			double eps = 1e-9;

			if (input.get_magnitude(counter, sec) >=
					input.get_magnitude(sec, counter)) {
				switch(type) {
					case TUP_TUP:
						score += bc;
						break;
					case TUP_SV:
						score += bc/(eps + ca);
						break;
					case TUP_MIN:
						score -= min(ab, bc) + eps * max(ab, bc);
						break;
					case TUP_ALT_1:
						score -= ca/(eps+bc);
						break;
					case TUP_ALT_2:
						score += (ab+eps) * (bc+eps) * (bc+eps);
						break;
					case TUP_ALT_3:
						score += sqrt(ab) + bc;
						break;
					case TUP_ALT_4:
						score += ab + bc * bc;
						break;
					case TUP_ALT_5:
						score += ab + 2 * bc;
						break;
					case TUP_ALT_6:
						score += pow(ab+eps, 3.9) + pow(bc+eps, 0.9);
						break;
					default:
						throw -1;
						break;
				}
/*				if (type == TUP_TUP)
					score += input.get_magnitude(sec, third_party);
				else
					score += (double)(input.get_magnitude(sec, third_party))/(1e-6 + input.get_magnitude(third_party, counter));*/
			}
		}

		out.insert(candscore(counter, score));
	}

	return(pair<ordering, bool>(out, false));
}
