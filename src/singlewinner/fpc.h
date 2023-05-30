// First preference Copeland: A candidate A's score is equal to
// the sum of the Plurality scores of the candidates who beat A
// pairwise, negated. Ties are broken by Plurality scores of the
// candidates who tie A.

// A Condorcet Winner gets a score of zero, so the method passes
// Condorcet.

// First source:
// http://lists.electorama.com/pipermail/election-methods-electorama.com/2006-December/117180.html

// Contrary to the source, first preference Copeland fails mono-raise:
//		 8: A>B>C
//		 2: A>C>B
//		 9: B>C>A
//		12: C>A>B  C wins
//	  Raise C on the 2 A>C>B ballots:
//		 8: A>B>C
//		 9: B>C>A
//		14: C>A>B  B wins

// It also fails clone independence (vote-splitting):
//		 6: A>B>C
//		 3: C>A>B
//		 4: B>C>A    A wins
//	  Clone A into A1, A2, A3:
//		 2: A1>A2>A3>B>C
//		 2: A2>A3>A1>B>C
//		 2: A3>A1>A2>B>C
//		 3: C>A2>A3>A1>B
//		 4: B>C>A3>A1>A2    C wins
// Source: http://lists.electorama.com/pipermail/election-methods-electorama.com/2006-December/084474.html

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

		std::string name() const {
			return ("First pref. Copeland");
		}
};

#endif