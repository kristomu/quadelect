#ifndef _VOTE_SW_EMETH
#define _VOTE_SW_EMETH

#include "../ballots.h"
#include "../tools.h"
#include "../cache.h"
#include <iostream>
#include <vector>
#include <list>

using namespace std;

// Note to self: A method returns a rated ordering if it is the case that
// for any possible input, a single voter can alter at least one of the scores
// by modifying his ballot.
// We might have multiple layers of this: The method returns a limited rating
// if the above is the case, and returns a greater rating if a voter can alter
// the score by at least (sum) * (1/number of voters). However, I have no way
// of proving either, so I'll go by intuition.
//	(For instance, does Kemeny return limited or greater ratings?)

// -------------- //

// The structure of the election_method class is as follows: the publically
// exposed elect functions first check if the cache has any records for this
// method. If it has, we just return it, blindly - the caller must make sure
// the cache is cleared between rounds - but if it isn't, we continue to the
// appropriate elect_inner function, which is what inherited functions usually
// specify. Finally, unless a tiebreaker has been used, the public function 
// dumps the output into the cache and also returns that output.

// If elect is called with a hopefuls boolean that is not all-true (i.e. some
// candidates are excluded), then that is in effect another ballot, and so the
// public elect functions don't check the cache (which is only used when no
// candidates are eliminated). This strategy makes elimination methods look in
// the cache for the first round of the base method, but not for subsequent 
// rounds.

// If winner_only is set, that means the caller only needs the first rank
// (winner or winners in case of a tie). It does no harm to return the actual
// social order, but if it is easier to determine the winner, do so. Note that
// in comma (x,y) methods, only the set can have winner_only; say the x set is
// {A,B} and y ranks A next to last and B last. Then the output is A>B, which
// we can't know unless y produces the full ordering.

// TODO also, encapsulate the cache in a class so that the coder doesn't have
// to be cautious of winner vs full ordering, etc. That would also let us cache
// Condorcet matrices later.

// Perhaps return ballot_group instead of ordering, so that we can set 
// rated/ranked according to what the method returns... Bluesky.

// BLUESKY: Do something that will make hopefuls obsolete, so that we can
// ensure that the method will only consider certain candidates, but will
// proceed as if that was a complete ballot set with only the hopefuls
// actually mentioned.

// ABC
class election_method {

	protected:
		// Use these when programming inherited classes. The cache
		// reference has to be read-write as some methods may add
		// additional information to it - for instance, the comma class
		// can add information from the base method and set.

		// The boolean should be true if only the winner was calculated,
		// otherwise false. Upon error, let the bool be undefined and
		// return an empty ordering.
		virtual pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				int num_candidates, cache_map & cache,
				bool winner_only) const;
		virtual pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls, 
				int num_candidates, cache_map & cache,
				bool winner_only) const = 0;

	public:
		// ElectEx? :p
		// The next two shouldn't be publicly used; instead, they're
		// used in combination methods that have to know if what they're
		// returning is winner_only or not.
		pair<ordering, bool> elect_detailed(const list<ballot_group> &
				papers, int num_candidates, cache_map & cache,
				bool winner_only) const;

		pair<ordering, bool> elect_detailed(const list<ballot_group> &
				papers, const vector<bool> & hopefuls,
				int num_candidates, cache_map & cache,
				bool winner_only) const;

		// Public wrappers for cache.
		ordering elect(const list<ballot_group> & papers, 
				int num_candidates, cache_map & cache,
				bool winner_only) const;
		// For elimination. Make better, later.
		ordering elect(const list<ballot_group> & papers, 
				const vector<bool> & hopefuls, 
				int num_candidates, cache_map & cache,
				bool winner_only) const;

		// Public wrappers for when there is no cache. These just
		// forward to the appropriate elect method with cache set to
		// NULL.
		ordering elect(const list<ballot_group> & papers,
				int num_candidates, bool winner_only) const {
			return(elect(papers, num_candidates, 
						*(cache_map *)(NULL), 
						winner_only)); }

		ordering elect(const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, bool winner_only) const {
			return(elect(papers, hopefuls, num_candidates, 
						*(cache_map *)(NULL),
						winner_only)); }

		// Here goes stats stuff like "returns rated vote" (score not
		// just rank), "returns complete ordering or just winners",
		// etc. Perhaps better would be to have this set by constructor
		// so we can make the derived constructors succinct.
		
		// Perhaps also complexity estimate (so we can determine on-the-
		// fly whether to include CPO-STV etc), and what criteria the
		// method is known to fail.
		
		virtual string name() const = 0;

		// Virtual destructor so delete removes inherited classes' data.
		virtual ~election_method() {}
};

#endif
