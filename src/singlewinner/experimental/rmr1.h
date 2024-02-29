#pragma once

// Resistant Mono-Raise attempt 1, the "bottom up" method:

// Starting with pairwise defeats, "make the case" for each candidate.

// First set the defeat graph so that a candidate has an edge pointed at
// him if someone else beats him pairwise.

// Then do the following for k = 3...n.

// For every candidate B who is defeating A according to the graph,
// check if A ~(k)~> B, i.e. if A disqualifies B on all sets of cardinality
// k. If so, reverse the direction of the edge between A and B.

// If A is now undefeated, return A's penalty as k. Otherwise, if k=n,
// return A's penalty as infinity (or n+1).

// The candidate with the least penalty wins. This method might not be
// resolvable; I'll check that later.

// A ~(k)~>B is defined as by the disqualification relation of the
// resistant/inner burial set.

// A lot of this is stolen from the resistant set code; I'll refactor
// later if this seems to show promise. TODO.

#include "../method.h"
#include "../../pairwise/matrix.h"
#include "../positional/simple_methods.h"

class rmr1 : public election_method {
	private:
		plurality plurality_method;

		std::vector<std::vector<std::vector<bool> > >
		get_k_disqualifications(const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "RMR1 (EXP)";
		}

		// I'm not sure which is better; IFPP Method X uses
		// whole, and this uses fractional.
		rmr1() : plurality_method(PT_FRACTIONAL) {}
};