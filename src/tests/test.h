#pragma once

#include <vector>

#include "cache.h"
#include "disproof.h"

#include "../ballots.h"

#include "../generator/ballotgen.h"
#include "../singlewinner/method.h"
#include "../random/random.h"

// The strategy (test) classes are ultimately meant to be designed like
// this:
//	They take an index from 0 to a maximum exclusive, and produce some
//	kind of test outcome. If the maximum is -1, then that means there
//  are so many that exhaustive traversal is intractable (wouldn't fit
//  an int64_t).
//  Then there's a method to produce a strategy by index (if the max
//  isn't -1) and one to produce a random strategy.

// Ultimately I want enough machinery to support going through a random
// permutation of these, as well as going through them exhaustively, and
// in such a way that the class doing the iteration (currently
// strategy_test) doesn't have to care about whether the strategy is
// cloning (candidate-centered) or burial/compromise or something else
// entirely... because I want to extend the strategy concept to tests in
// general.

// But I'll have to see if it's doable.

class criterion_test {
	protected:
		// Returns a partially formed disproof - I'm going to do it
		// like that and then profile to see if it's too slow.
		virtual void add_strategic_election_inner(
			disproof & partial_disproof, int64_t instance_index,
			const test_cache & cache, size_t numcands,
			pure_ballot_generator * ballot_generator,
			rng * randomizer) const = 0;

	public:
		// Check if the cache has the data we'll rely on; if not,
		// add it. The partial disproof must contain everything
		// pertaining to the honest outcome. (I might change this
		// later, it's sort of ugly...)
		void prepare_cache(test_cache & election_data,
			disproof & partial_disproof, size_t numcands) const;

		// This is a simple wrapper to ensure prepare_cache is always
		// called.
		void add_strategic_election(disproof & partial_disproof,
			int64_t instance_index, test_cache & cache,
			size_t numcands, pure_ballot_generator * ballot_generator,
			rng * randomizer) {

			prepare_cache(cache, partial_disproof, numcands);
			add_strategic_election_inner(partial_disproof,
				instance_index, cache, numcands,
				ballot_generator, randomizer);
		}

		virtual std::string name() const = 0;

		// "Monotonicity", "Strategy", etc.
		virtual std::string category() const = 0;

		virtual int64_t get_num_tries(size_t numcands) const = 0;

		// Returns true if the disproof shows a failure, false
		// otherwise. If this strategy isn't responsible for the
		// disproof, it'll throw an exception as that's not expected
		// to happen. (Fix this later if required)
		virtual bool is_disproof_valid(
			const disproof & disproof_to_verify) const;

		virtual void print_disproof(
			const disproof & disproof_to_print) const;
};