// "Mode" ratings, to complete the statistical methods.
// This method finds the candidate with the highest mode (most frequently
// occurring value). Ties are broken by a leximax procedure: if most frequently
// occurring value is the same, look at next-to-most frequent value, then next
// to next, etc.
// It's indecisive and has monotonicity problems: moving someone's rating from,
// say, 4 to 5 might cause that candidate's mode to go from 4 to 3.

// No normalization yet. I should split that off elsewhere. Because of histogram
// artifacts, this method will probably do better with very wide bins, and a
// monotone version might make use of successive refinement of the bins, though
// that would look very much like Approval voting.

#ifndef _VOTE_SW_MODER
#define _VOTE_SW_MODER

#include "../method.h"
#include "tools/tools.h"
#include "common/ballots.h"


class mode_ratings : public election_method {

	private:
		std::string cached_name;

	protected:

		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string determine_name() const {
			return ("Mode-Ratings");
		}

	public:

		std::string name() const {
			return (cached_name);
		}

		mode_ratings() {
			cached_name = determine_name();
		}

};

#endif
