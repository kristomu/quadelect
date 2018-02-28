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

#include "../../method.h"
#include "../../../tools.h"
#include "../../../ballots.h"

#include "../../../grad_median/grad_median.h"

#include "comparand.h"

using namespace std;

class vi_median_ratings : public election_method {

	private:
		grad_fracile aggregate_ratings(const list<ballot_group> & 
				papers, int num_candidates, const vector<bool> &
				hopefuls, bool do_norm, double minimum,
				double maximum) const;

		pair<ordering, bool> get_ranks(grad_fracile & source,
				double fracile, bool tiebreak, 
				bool winner_only, bool debug) const;

		string cached_name;
		bool use_tiebreak, normalize;
		int med_maximum; // minimum is 0.

	protected:

		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, cache_map * cache,
				bool winner_only) const;

		string determine_name() const;

	public:

		string name() const { return(cached_name); }

		vi_median_ratings() { med_maximum = 10; use_tiebreak = false;
			normalize = false; cached_name = determine_name(); }

		vi_median_ratings(int max_in, bool norm_in, bool tiebreak_in){
			med_maximum = max_in; use_tiebreak = tiebreak_in;
			normalize = norm_in; cached_name = determine_name(); }
};

#endif
