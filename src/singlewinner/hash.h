// This method performs a hashing operation on the input ballot set, and
// orders the winners according to the output.
// It thus fails just about every property (as well as neutrality and
// symmetry) and is maximally manipulable. It's basically random
// candidate without actual randomness.
// The only point of the method is to prove lower bounds about
// compositions, e.g. the maximal manipulability of something that
// elects from the resistant set.

#pragma once

#include "method.h"
#include "lib/spookyhash/SpookyV2.h"

class hash_random_cand : public election_method {

	private:
		mutable SpookyHash hasher;

		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

	public:
		std::string name() const {
			return "Hash-random candidate";
		}
};