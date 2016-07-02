
// This one requires both an ordering and the Condorcet matrix, and therefore
// isn't a pairwise_method.

// Election method: Single elimination tournament. First seed the candidates
// according to the ordering given. Then pair up candidates, best against
// worst. The system is recursive, so that if you have 16 candidates, the 8
// that proceed are treated as the inputs to an 8-candidate method. 

// Since a single run through the tournament structure provides exponentially
// many ties at each level (those who lasted until that level), we need to run
// "loser brackets" to determine the ordering beyond first place. This adds up
// to log_2(n) runs.

// It breaks (assertion failure) on a tie. Should probably do something about
// that.

// TODO (BLUESKY): In the same vein, have Kemenize, Insertion-sort-ize,
// mergesort, etc. Init according to seeding, then sort based on the method in
// question, then return.

#ifndef _VOTE_P_TOURNAMENT
#define _VOTE_P_TOURNAMENT

#include "../../pairwise/matrix.h"
#include "../method.h"

#include <assert.h>
#include <iostream>

using namespace std;

class cond_tournament : public election_method {

	private:
		election_method * base; // used to find the seed order
		//bool one_round_only;

		// if a or b > size, then other always wins (simulates a bye)
		// 1 is beats, 0 tie, -1 loses
		int a_beats_b(int a, int b, const condmat & to_check) const;

		// This is used to recurse in order to get the winner.
		int get_victor(int check_a, int check_b, int level, int
				numcands, const vector<int> & seed_order,
				const condmat & to_check, vector<int> &
				lasted_how_long) const;

		// We need to run it multiple times to get a full ordering.
		// Not yet, though; gotta crawl etc.
		ordering internal_elect(const list<ballot_group> & papers,
				const condmat & matrix, const ordering & seed,
				const vector<bool> & hopefuls, int 
				num_candidates, bool winner_only) const;

	protected:
		cond_tournament(election_method * base_method, 
				bool single_round);
		// TODO, make these into elect_inner.
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const ordering & seed, const condmat & 
				matrix, bool winner_only) const;
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, cache_map & cache,
				bool winner_only) const;
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				int num_candidates, cache_map & cache,
				bool winner_only) const;

	public:
		string name() const;
};

#endif
