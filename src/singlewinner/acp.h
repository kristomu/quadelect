// Kevin Venzke's Adjusted Condorcet Plurality method.
// https://www.votingmethods.net/lnharm/

// First we find the plurality winner (call him X), then we create a
// derived ballot set where every voter's ballot is truncated below X,
// and then if the Condorcet matrix based on this derived ballot set
// has a Condorcet winner, that candidate is elected, otherwise X is.

// Equal-rank counts fractionally, and ties for first place are
// handled by producing a tie of every winner produced by doing this
// for each of the tied winners. (This may be very slow.)

// As Kevin's page states that "Analogous variants of ACP based on TTR
// are also possible", I've added the ability to specify what the base
// method should be (the one that picks the winner that ballots are
// truncated below). Choosing Plurality as the base method produces
// Adjusted Condorcet Plurality, so that's what the default constructor
// does.

// TODO? Implement the FPTP-flavored version where the Condorcet winner
// is found based on both the truncated and ordinary ballot set?

// Note that ACP(X) may sometimes weaken strategy resistance, e.g. when
// the base method X is IRV. But it often improves it.

#pragma once

#include "method.h"
#include "sets/condorcet.h"
#include "positional/simple_methods.h"

#include <memory>

class adjusted_cond_plur : public election_method {

	private:
		condorcet_set condorcet;
		std::shared_ptr<election_method> base_method;

		int get_adjusted_winner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			size_t base_winner,
			size_t num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "Adjusted Condorcet " + base_method->name();
		}

		adjusted_cond_plur() {
			base_method = std::make_shared<plurality>(
					plurality(PT_FRACTIONAL));
		}

		// Used for the TTR variant where (I imagine?) the top-two
		// winner is used as the threshold candidate. And since I
		// can make it generic, why not? Try IRV-ACP for instance...
		adjusted_cond_plur(
			std::shared_ptr<election_method> base_method_in) {
			base_method = base_method_in;
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