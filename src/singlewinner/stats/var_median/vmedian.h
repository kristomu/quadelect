// Median ratings, including the gradual relaxation tiebreaker specified in
// http://wiki.electorama.com/wiki/Median_Ratings . The tiebreaker handles
// weighted ballots with a plane-sweep algorithm.

// The idea of this algorithm is to keep two vertical lines at 50% +/- p,
// p increasing. The vertical lines stop at the closest transition between
// two piecewise constant segments of the sorted ratings function.

// TODO: more here. Add tests in another file.
// Perhaps "Central Tendency Ratings" or somesuch?

#ifndef _VOTE_SW_VMEDIAN_II
#define _VOTE_SW_VMEDIAN_II

#include <algorithm>
#include <iterator>

#include <assert.h>

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <map>
#include <set>

#include <math.h>

#include "singlewinner/method.h"
#include "tools/tools.h"
#include "common/ballots.h"

#include "grad_median/grad_median.h"

#include "comparand.h"


class vi_median_ratings : public election_method {

	private:
		grad_fracile aggregate_ratings(const election_t &
			papers, int num_candidates, const std::vector<bool> &
			hopefuls, bool do_norm, double minimum,
			double maximum) const;

		std::pair<ordering, bool> get_ranks(grad_fracile & source,
			double fracile, bool tiebreak,
			bool winner_only, bool debug) const;

		std::string cached_name;
		bool use_tiebreak, normalize;
		int med_maximum; // minimum is 0.

	protected:

		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string determine_name() const;

	public:

		std::string name() const {
			return (cached_name);
		}

		vi_median_ratings() {
			med_maximum = 10; use_tiebreak = false;
			normalize = false; cached_name = determine_name();
		}

		vi_median_ratings(int max_in, bool norm_in, bool tiebreak_in) {
			med_maximum = max_in; use_tiebreak = tiebreak_in;
			normalize = norm_in; cached_name = determine_name();
		}
};

#endif
