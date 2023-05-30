// The "tup" method and a similar Tup/SV method.
// If there are more than three candidates, abort (undefined). Otherwise,
// A's score is B>C (If A beats B) + C>B (if A beats C).
// Same for the other candidates.
// Tup/SV: A's score is B>C/(epsilon+C>A) instead. Ideally, take the rank as
// epsilon->0.

#ifndef _VOTE_P_TUP
#define _VOTE_P_TUP

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

enum tup_type { TUP_TUP, TUP_SV, TUP_MIN, TUP_ALT_1, TUP_ALT_2, TUP_ALT_3,
	TUP_ALT_4, TUP_ALT_5, TUP_ALT_6
};

class tup : public pairwise_method {
	private:
		tup_type type;

	public:
		std::pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		std::string pw_name() const {
			switch (type) {
				case TUP_TUP: return ("EXP:Tup");
				case TUP_SV: return ("EXP:Tup-SV");
				case TUP_MIN: return ("EXP:Tup-Min");
				case TUP_ALT_1: return ("EXP:Tup(1)");
				case TUP_ALT_2: return ("EXP:Tup(2)");
				case TUP_ALT_3: return ("EXP:Tup(3)");
				case TUP_ALT_4: return ("EXP:Tup(4)");
				case TUP_ALT_5: return ("EXP:Tup(5)");
				case TUP_ALT_6: return ("EXP:Tup(6)");
				default: return ("EXP:Tup(\?\?\?)");
			}
		}

		tup(pairwise_type def_type_in,
			tup_type tup_type_in):pairwise_method(def_type_in) {
			type = tup_type_in;
			update_name();
		}

};

#endif
