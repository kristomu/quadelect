#ifndef _VOTE_SW_CARDR
#define _VOTE_SW_CARDR

#include "../method.h"
#include "../../tools.h"
#include "../../ballots.h"

using namespace std;

// Cardinal Ratings (Range).

class cardinal_ratings : public election_method {

	private:

		string cached_name;

		// Normalization parameters. If normalize is off, values above
		// or below are clamped to the maximum (or minimum, 
		// respectively).
		bool normalize; // Should values be normalized to range?
		int minimum, maximum; // And what is that range?

		vector<double> aggregate_ratings(const list<ballot_group> &
				papers, int num_candidates, 
				const vector<bool> & hopefuls) const;

	protected:

		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates,
				cache_map & cache, bool winner_only) const;

		string determine_name() const;

	public:

		string name() const { return(cached_name); }

		cardinal_ratings(int min_in, int max_in, bool norm_in);
};

#endif
