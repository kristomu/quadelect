#pragma once

// Slash - meta-method with which to build methods like Smith/IRV.
// The meta-method first calculates the outcome of the first method (which is
// usually a set, like Smith), and then calculates the outcome of the second,
// constrained to the set of winners according to the first method. Then the
// outcome is the result of the second method, with ties broken according to
// the first.
// NOTE: There may be bugs in how hopefuls work for various methods. Fix later.
// BLUESKY: Iterated. E.g. say Smith returns A = B = C > D = E = F > G = H = I
//	then the result is the outcome for {A,B,C} with ties broken by the
//	outcome for {A,B,C,D,E,F} and so on.

#include "../method.h"
#include "../../ballots.h"
#include "../../tools/ballot_tools.h"

#include <memory>

class slash : public election_method {

	private:
		std::shared_ptr<const election_method> set_method;
		std::shared_ptr<const election_method> specific_method;

	protected:
		std::pair<ordering, bool> elect_inner(const election_t &
			papers, const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

	public:

		// Now the constructor is in the intuitive order.
		slash(std::shared_ptr<const election_method> set_in,
			std::shared_ptr<const election_method> specific_method_in) {
			set_method = set_in;
			specific_method = specific_method_in;
		}

		std::string name() const;
};