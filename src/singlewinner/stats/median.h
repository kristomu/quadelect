// Median ratings, the median version of Cardinal Ratings (range). We also
// suport the "gradual relaxation" tiebreak specified in
// http://wiki.electorama.com/wiki/Median_Ratings.

// Maybe we should move normalization elsewhere. Hm.

// UNTESTED

#ifndef _VOTE_SW_MEDIAN
#define _VOTE_SW_MEDIAN

#include "../method.h"
#include "tools/tools.h"
#include "common/ballots.h"


class median_ratings : public election_method {

	private:

		std::string cached_name;
		bool tiebreak;

		// Normalization parameters. Like Cardinal Ratings, these decide
		// whether the least ballot value is pulled to minimum and the
		// greatest to maximum, and also what the range is. E.g. median
		// approval is Median Ratings-2.
		bool normalize;
		int range_minimum, range_maximum;

		double get_trunc_mean_destructively(
			std::vector<std::pair<double, double> > & array,
			double num_voters, bool already_sorted,
			double distance) const;

		std::vector<double> aggregate_ratings(const election_t &
			papers, int num_candidates, const std::vector<bool> &
			hopefuls) const;

	protected:

		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates,
			cache_map * cache, bool winner_only) const;

		std::string determine_name() const;

	public:

		std::string name() const {
			return (cached_name);
		}

		median_ratings(int min_in, int max_in, bool norm_in,
			bool tiebreak_in);

};

#endif