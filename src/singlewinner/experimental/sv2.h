#ifndef _VOTE_SV_SEC
#define _VOTE_SV_SEC

#include "../../pairwise/matrix.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>

#include <assert.h>

using namespace std;

class sv_att_second : public election_method {

	public:
		pair<ordering, bool> elect_inner(
			const list<ballot_group> & papers,
			const vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		string name() const {
			return ("EXP:SV Attempt 2");
		}
};

#endif
