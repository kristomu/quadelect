#pragma once

// The resistant or inner burial set winner analog.
// See inner_burial.h for more info about the set.

// This winner set consists of A if A disqualifies everybody else.
// If there is no such A, then the set is the set of all the hopefuls.

// This set notion is directly monotone and it would be interesting to
// see just how much of the resistant set's strategy resistance we can
// retain. If we get most of it, then it might be a good start to try
// to devise clone independence, etc., from; alternatively, to try to
// capture the logic that makes fpA-fpC resistant and monotone.

#include "../method.h"
#include "../../pairwise/matrix.h"
#include "../positional/simple_methods.h"


class resistant_winner_set : public election_method {
	private:
		plurality plurality_method;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Resistant winner";
		}

		// I'm not sure which is better; IFPP Method X uses
		// whole, and this uses fractional.
		resistant_winner_set() : plurality_method(PT_FRACTIONAL) {}
};