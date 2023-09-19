#ifndef _VOTE_EXP_FPA
#define _VOTE_EXP_FPA

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>
#include <assert.h>


class fpa_experiment : public election_method {
	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return ("EXP:fpA-experiment/Simmons");
		}
};

#endif