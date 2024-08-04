#pragma once
#include <list>
#include "common/ballots.h"

// This is the relevant cache data structure for tests that relate to
// strategic manipulation, where voters who prefer some candidate X to the
// current winner W change their ballots to make X win instead of W.
// The candidate X is called the "challenger".

// Since we're trying to engineer a strategy in favor of the challenger,
// we divide the ballots into those that explicitly support the challenger,
// and everybody else. Those that are indifferent to whether the winner or
// challenger wins thus don't count as supporting the challenger.
class ballots_by_support {
	public:
		election_t supporting_challenger,
				   others;

		double challenger_support, other_support;
		size_t winner, challenger;

		void reset(size_t winner_in, size_t challenger_in) {
			challenger_support = 0;
			other_support = 0;
			winner = winner_in;
			challenger = challenger_in;
			supporting_challenger.clear();
			others.clear();
		}

		ballots_by_support(size_t winner_in, size_t challenger_in) {
			reset(winner_in, challenger_in);
		}

		// Initialize data according to a given set of ballots.
		void group_by_support(
			const election_t & ballots,
			size_t winner_in, size_t challenger_in);

		ballots_by_support(const election_t & ballots,
			size_t winner_in, size_t challenger_in) {

			group_by_support(ballots, winner_in, challenger_in);
		}
};