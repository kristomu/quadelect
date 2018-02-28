// Random ballot (or "random dictator"). The method is strategy-proof.

#ifndef _VOTE_SW_RANDBALL
#define _VOTE_SW_RANDBALL

#include "../method.h"
#include "../../tools.h"
#include "../../ballots.h"

using namespace std;

class random_ballot : public election_method {

	protected:
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates,
				cache_map * cache, bool winner_only) const;

	public:

		string name() const { return ("Random Ballot"); }
};

#endif
