// Kevin Venzke's Adjusted Condorcet Plurality method.
// https://www.votingmethods.net/lnharm/

// First we find the plurality winner (call him X), then we create a
// derived ballot set where every voter's ballot is truncated below X,
// and then if the Condorcet matrix based on this derived ballot set
// has a Condorcet winner, that candidate is elected, otherwise X is.

// Equal-rank counts fractionally, and ties for first place are
// handled by producing a tie of every winner produced by doing this
// for each of the tied winners. (This may be very slow.)

// TODO? Implement the FPTP-flavored version where the Condorcet winner
// is found based on both the truncated and ordinary ballot set?

#pragma once

#include "method.h"
#include "sets/condorcet.h"

class adjusted_cond_plur : public election_method {

	private:
		condorcet_set condorcet;

		int get_adjusted_winner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			size_t plurality_winner,
			size_t num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "Adjusted Condorcet Plurality";
		}
};

/* TODO once I have tests going:

16: A > B > C
19: A > C > B
18: C > A > B
16: B > C > A
20: B > C > A
8: C > B > A

The winner should be A with truncated ballots

16: A > B
19: A > C > B
18: C > A > B
16: B
20: B
8: C > B */