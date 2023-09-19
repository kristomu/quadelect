// The "IFPP-like" extension of fpA-fpC listed on
// https://electowiki.org/wiki/FpA-fpC

// If A and B both have more than 1/3 of first preferences, then elect
// the candidate who beats the other pairwise. Otherwise, elect the
// Plurality winner.

// We make no attempts to create a score, just an ordering. Scores may
// come later; the intuitive idea would be for the first two candidates
// A and B to have A>B and B>A as scores if they both exceed 1/3 first
// prefs., but this is not guaranteed to be greater than the other
// candidates' first preferences if truncation is going on.

// The Plurality count is fractional, i.e. equal rank gives each
// candidate 1/n of a vote. Other types may retain burial resistance;
// check later.

#pragma once

#include "../method.h"

class ifpp_like_fpa_fpc : public election_method {

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "IFPP-like fpA-fpC";
		}
};