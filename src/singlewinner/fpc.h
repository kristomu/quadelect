// First preference Copeland: A candidate A's score is equal to
// the sum of the Plurality scores of the candidates who beat A
// pairwise, negated. Ties are broken by Plurality scores of the
// candidates who tie A.

// A Condorcet Winner gets a score of zero, so the method passes
// Condorcet.

// Needs to be verified against the proper 3 candidate linbrute
// just for testing purposes.

#ifndef _VOTE_FPC
#define _VOTE_FPC

#include "method.h"
#include "../pairwise/matrix.h"
#include "positional/simple_methods.h"

class first_pref_copeland : public election_method {
		
	public:
		std::pair<ordering, bool> elect_inner(
				const std::list<ballot_group> & papers,
				const std::vector<bool> & hopefuls,
				int num_candidates, cache_map * cache,
				bool winner_only) const;

		string name() const { return ("First pref. Copeland"); }
};

#endif