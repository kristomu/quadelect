// Majority Defeat Disqualification set. If A is preferred to B on a majority
// of the ballots, then B is "disqualified" (in this case, ranked below top).
// Our ranking may be slightly different from the usual Majority Defeat
// Disqualification concept, since if everybody is thus disqualified, people
// who are disqualified by more than one will still be ranked below those only
// disqualified by one. Set sum to false if you don't want that.

// A majority ranks A above B if A has a majority strength pairwise victory
// over B. Therefore, we must use WV (or MARGINS, etc; not PO, KEENER, etc).

#ifndef _SET_MDD
#define _SET_MDD

#include "../method.h"
#include "../pairwise/method.h"

class mdd_set : public pairwise_method {
	private:
		bool sum_defeats;

	public:
		std::pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;

		mdd_set(bool sum_defeats_in);

		std::string pw_name() const;
};

#endif
