// Kevin Venzke's Quick Runoff method,
// https://electowiki.org/wiki/Quick_Runoff

// There are two notable differences between this implementation and
// Kevin's definition.

// First, I don't handle ties yet. (FIX LATER, TODO). Ties could lead
// to a Ranked Pairs-type problem where every permutation of the tied
// candidates has to be checked to determine if that candidate can win.
// So that's understandably too difficult for now, although it means
// that the method isn't yet usable for "serious" cases.

// Second, I do equal-rank by using a fractional Plurality count. I
// don't know the implications of this on LNHarm.

// It also doesn't support a social ordering; the outcome will always
// consist of just a winner, with everybody else tied for last. I need
// to find a natural extension before I add a full social ordering.

#pragma once

#include "method.h"

class quick_runoff : public election_method {

	private:
		// This returns an ordering with the winner first
		// and everybody else tied for second.
		ordering winner_to_ordering(size_t winner,
			const std::vector<bool> & hopefuls,
			size_t num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "Quick Runoff";
		}
};