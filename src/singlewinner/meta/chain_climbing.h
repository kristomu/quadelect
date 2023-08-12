#pragma once

#include "../method.h"
#include "../sets/condorcet.h"

#include <memory>

// This is an implementation of Jobst Heitzig's chain climbing idea.
// It takes another method as base method, and then, starting with the loser,
// repeatedly adds in higher-ranked candidates as long as the candidate to
// be added beats every candidate already added. If the candidate doesn't,
// then the method ends with the last added candidate being the winner.

// Tiebreaks are handled by recursing over every order and using the
// first seen tiebreaker on them, which in practice means every candidate
// who can be a winner by breaking the tie in some order.

// Since it's far from obvious how to create a social ordering, I don't;
// the final candidate beating everybody else is a winner, and everybody
// else is a loser. Potential ways to extend this include ignoring the
// winner and continuing on; whoever then wins is second, etc.

// However, I am not able to do *quite* enough levels of indirection in
// my head to implement that. So winner-only it is, for now. Hence I'm just
// copy-pasting a bunch of code from Benham. There might be some underlying
// common pattern to exploit to them, which would let me make a social
// ordering method for both if I can do it for one... but TODO. If ever.

class chain_climbing : public election_method {

	private:
		std::shared_ptr<election_method> base_method;

		void determine_winners(
			const condmat & condorcet_matrix,
			const std::vector<bool> & hopefuls,
			std::vector<bool> & winners_so_far,
			ordering remaining_base_ordering,
			ordering current_chain) const;

		std::vector<bool> get_winners(
			const std::list<ballot_group> & election,
			const std::vector<bool> & hopefuls,
			ordering base_method_ordering) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "[" + base_method->name() + "]-Chain Climbing";
		}

		chain_climbing(
			std::shared_ptr<election_method> base_method_in) {

			base_method = base_method_in;
		}

		// This is rather ugly and not at all how you're supposed
		// to create shared pointers, but keep it for now while the
		// rest of quadelect uses bare pointers.
		chain_climbing(
			election_method * base_method_in) {

			base_method = std::shared_ptr<election_method>(
					base_method_in);
		}
};