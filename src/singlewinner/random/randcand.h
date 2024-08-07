// Random candidate. Simply take the available hopeful candidates, shuffle them,
// and output in order. It's obviously not cloneproof.

// This method shows why we should have a "result happened due to chance" output
// indicator. If we run a strategy test on this, it will say that the strategy
// always succeeds because if you run it for long enough, the candidate one's
// strategizing in favor of wins anyway, but the method is really strategy-proof
// (because it doesn't look at the ballots at all!)

// In a dream world, ties or probabilistic methods would return the probability
// of each candidate winning, so that the Yee functions could draw just the
// right combination of colors and tests could handle probabilistic monotonicity
// failure etc.; but for fractal methods like IRV, it's hard to know the probs.

#ifndef _VOTE_SW_RANDCAND
#define _VOTE_SW_RANDCAND

#include "../method.h"
#include "tools/tools.h"
#include "common/ballots.h"


class random_candidate : public election_method {

	protected:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates,
			cache_map * cache, bool winner_only) const;

	public:

		std::string name() const {
			return ("Random Candidate");
		}
};

#endif
