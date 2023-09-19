// The "first seen" tiebreaker method. Given an election that represents a
// list of outcomes from different ways to break ties in some other method,
// it constructs a composite outcome like this:

// Every candidate who is ranked first on at least one of the ballots tie
// for first. Then every candidate who is ranked second on at least one of
// the ballots, and who has not already been listed, goes second,
// then every candidate ranked third, and so on.

#pragma once

#include "../method.h"

class first_seen_tiebreak : public election_method {

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return ("First seen tie-aggregator");
		}
};