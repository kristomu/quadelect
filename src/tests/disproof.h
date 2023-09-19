#pragma once

#include <vector>
#include <string>
#include "../ballots.h"

// The disproof class contains all information required to prove a failure of
// some criterion.

// The disproof class assumes that we're dealing with a dynamic
// criterion, i.e. where there's a before and after situation.
// This includes things like monotonicity and strategic manipulation.
// Static criteria are those where just one election is required
// to determine pass/fail, e.g. reversal symmetry. I'll implement those
// later as needed, and possibly exotic stuff like consistency too, which
// has multiple "after" elections.

// The additional informations map is kind of a poor man's dynamic typing
// hack, but it's necessary because the function iterating over the
// different tests can't tell different types of disproof apart due to the
// inheritance structure. So every disproof has to have the
// capability to store information needed for any particular disproof
// type.

class disproof {
	public:
		election_t before_election, after_election;
		ordering before_outcome, after_outcome;
		std::map<std::string, size_t> data;
		std::string disprover_name;

		std::string name() const {
			return disprover_name;
		}
};