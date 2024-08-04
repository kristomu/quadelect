#pragma once

#include "../method.h"
#include "common/ballots.h"
#include "tools/tools.h"
#include "tools/ballot_tools.h"

#include <list>
#include <memory>

/* This file implements a variant of Forest Simmons' CTE method.
   His method is defined as follows:

1. Elect the undefeated (remaining) candidate when there is one ... else ...
	eliminate any candidate that defeats no other (remaining) candidate ...
       	and ...
2. List the remaining candidates in order of [the base method's evaluation].
3. Update the list by [Bubble] Sorting it pairwise.
4. Let P (for Pivot) be the candidate that has sunk to the bottom of the list.
5. Eliminate P along with any candidates defeated by P.
6. While more than one candidate remains, repeat steps 2 through 5.
7. Elect the uneliminated candidate.

This method does the same, except it repeats from step one. */

class cte : public election_method {
	private:
		std::shared_ptr<const election_method> base;

	protected:
		std::pair<ordering, bool> elect_inner(const
			election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

	public:

		cte(std::shared_ptr<const election_method> base_method) {
			base = base_method;
		}

		std::string name() const {
			return "EXP: CTE/" + base->name();
		}
};
