#pragma once

#include "random/random.h"
#include "methods.h"
#include <list>

// "Random ballots": shuffle the ballots, then elect a random first preference from
// each voter until the council is full or we've exhausted the ballots. If it's the
// latter, do it again but ignore the already chosen candidates.

// Note that this is different from random dictator, which elects a slate of a
// random voter's preferences.

// I don't really think there's a reason to pass general coordinate generators
// into this method because I don't see how QMC would help. Maybe if this is run
// tons of times?

class random_ballots : public multiwinner_method {
	private:
		mutable rng random_gen;

	public:
		std::list<int> get_council(int council_size,
			int num_candidates, const election_t & ballots) const;

		std::string name() const {
			return "Random ballots";
		}

		void set_seed(rseed_t seed) {
			random_gen.s_rand(seed);
		}

		random_ballots(rseed_t seed) : random_gen(seed) {}
};