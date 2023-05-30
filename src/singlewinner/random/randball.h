// Random ballot (or "random dictator"). The method is strategy-proof.

#ifndef _VOTE_SW_RANDBALL
#define _VOTE_SW_RANDBALL

#include "../method.h"
#include "../../tools/tools.h"
#include "../../ballots.h"


class random_ballot : public election_method {

	protected:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates,
			cache_map * cache, bool winner_only) const;

	public:

		std::string name() const {
			return ("Random Ballot");
		}
};

#endif
