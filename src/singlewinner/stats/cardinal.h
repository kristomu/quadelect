#ifndef _VOTE_SW_CARDR
#define _VOTE_SW_CARDR

#include "../method.h"
#include "../../tools/tools.h"
#include "../../ballots.h"


// Cardinal Ratings (Range).

class cardinal_ratings : public election_method {

	private:

		std::string cached_name;

		// Normalization parameters. If normalize is off, values above
		// or below are clamped to the maximum (or minimum,
		// respectively).
		bool normalize; // Should values be normalized to range?
		int minimum, maximum; // And what is that range?

		std::vector<double> aggregate_ratings(const std::list<ballot_group> &
			papers, int num_candidates,
			const std::vector<bool> & hopefuls) const;

	protected:

		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates,
			cache_map * cache, bool winner_only) const;

		std::string determine_name() const;

	public:

		std::string name() const {
			return (cached_name);
		}

		cardinal_ratings(int min_in, int max_in, bool norm_in);
};

#endif
