#ifndef _VOTE_EXP_FPA
#define _VOTE_EXP_FPA

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>
#include <assert.h>

using namespace std;

class fpa_experiment : public election_method {
	public:
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, cache_map * cache,
				bool winner_only) const;

		string name() const {
			return ("fpA-experiment");
		}
};

#endif