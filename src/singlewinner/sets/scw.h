// Francois Durand's "Super Condorcet winner": the candidate
// (if any) who, in resistant set terms, disqualifies everybody
// else directly.

// I could build on this if necessary.

// Hold on... can this even work? Suppose A~>B, A~>C. Couldn't B>A
// voters possibly break A~>C? Say they bury C: BAC -> BCA. This
// breaks A~>C and now A is no longer the Super CW...

#pragma once

#include "../method.h"

class super_condorcet_set : public election_method {
	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Super Condorcet";
		}
};
