#pragma once

// This is the Benham wrapper, which repeatedly removes candidates from
// the end of the base method's ordering until the remaining candidates
// have a Condorcet winner. Note that this does not do any elimination
// itself, so what's called Benham's method would in this context be
// Benham-IRV. This is done so that I can test things like Plurality
// Benham. I may change this later because it isn't very intuitive.

// If there's a tie, the method will descend down all ways to break the
// tie. Every candidate that can be turned into a CW this way share the
// first place in the output ordering, then every other candidate follows
// according to the original ordering.

#include <memory>

#include "../method.h"
#include "../sets/condorcet.h"

class benham_meta : public election_method {

	private:
		std::shared_ptr<election_method> base_method;
		condorcet_set condorcet;

		void determine_winners(
			const condmat & condorcet_matrix,
			std::vector<bool> & remaining_candidates,
			std::vector<bool> & winners_so_far,
			ordering remaining_ordering) const;

		std::vector<bool> get_winners(
			const std::list<ballot_group> & election,
			std::vector<bool> hopefuls,
			ordering base_method_ordering) const;

	public:
		std::pair<ordering, bool> elect_inner(const std::list<ballot_group> &
			papers, const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Benham-Meta[" + base_method->name() + "]";
		}

		benham_meta(std::shared_ptr<election_method> base_method_in) {
			base_method = base_method_in;
		}

		benham_meta(election_method * base_method_in) {
			base_method = std::shared_ptr<election_method>(base_method_in);
		}
};