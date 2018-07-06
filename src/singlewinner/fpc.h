// First preference Copeland: A candidate A's score is equal to
// the sum of the Plurality scores of the candidates who beat A
// pairwise, negated. Ties are broken by Copeland score.

// A Condorcet Winner gets a score of zero, so the method passes
// Condorcet.

// If we clone X into X1 and X2, then anyone beaten by X will be
// beaten by both X1 and X2 and get the same deduction of their
// score as if X wasn't cloned. But if X is the winner, cloning X
// might lead both X1 and X2 to lose.

#ifndef _VOTE_FPC
#define _VOTE_FPC

#include "method.h"
#include "../pairwise/matrix.h"
#include "positional/simple_methods.h"

class first_pref_copeland : public election_method {
		
	public:
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, cache_map * cache,
				bool winner_only) const;

		string name() const { return ("First pref. Copeland"); }
};

#endif