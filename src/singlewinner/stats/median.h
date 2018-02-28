// Median ratings, the median version of Cardinal Ratings (range). We also
// suport the "gradual relaxation" tiebreak specified in 
// http://wiki.electorama.com/wiki/Median_Ratings.

// Maybe we should move normalization elsewhere. Hm.

// UNTESTED

#ifndef _VOTE_SW_MEDIAN
#define _VOTE_SW_MEDIAN

#include "../method.h"
#include "../../tools.h"
#include "../../ballots.h"

using namespace std;

class median_ratings : public election_method {

	private:

		string cached_name;
		bool tiebreak;

		// Normalization parameters. Like Cardinal Ratings, these decide
		// whether the least ballot value is pulled to minimum and the
		// greatest to maximum, and also what the range is. E.g. median
		// approval is Median Ratings-2.
		bool normalize;
		int range_minimum, range_maximum;

		double get_trunc_mean_destructively(
				vector<pair<double, double> > & array,
				double num_voters, bool already_sorted,
				double distance) const;

		vector<double> aggregate_ratings(const list<ballot_group> &
				papers, int num_candidates, const vector<bool> &
				hopefuls) const;

	protected:

		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates,
				cache_map & cache, bool winner_only) const;

		string determine_name() const;

	public:

		string name() const { return(cached_name); }

		median_ratings(int min_in, int max_in, bool norm_in, 
				bool tiebreak_in);

};

#endif