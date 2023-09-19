#ifndef _VOTE_SV_SEC
#define _VOTE_SV_SEC

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>

#include <assert.h>


class sv_att_second : public election_method {

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return ("EXP:SV Attempt 2");
		}
};

#endif
