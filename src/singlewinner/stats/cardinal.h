#pragma once

#include "../method.h"
#include "../../tools/tools.h"
#include "../../ballots.h"


// Cardinal Ratings (Range).

class cardinal_ratings : public election_method {

	private:

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

	public:

		cardinal_ratings(int min_in, int max_in, bool norm_in);

		std::string name() const;
};