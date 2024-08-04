#pragma once

// Comma - meta-method with which to build methods like Smith,IRV.
// The meta-method first calculates the outcome of the first method (which is
// usually a set, like Smith), and then breaks the "ties" - tiers of set
// membership - according to the second method.

#include "../method.h"
#include "common/ballots.h"
#include "tools/ballot_tools.h"

#include <memory>

class comma : public election_method {

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
		comma(std::shared_ptr<const election_method> set_in,
			std::shared_ptr<const election_method> specific_method_in) {
			set_method = set_in;
			specific_method = specific_method_in;
		}

		std::string name() const;
};