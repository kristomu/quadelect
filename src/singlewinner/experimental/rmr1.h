#pragma once

// Resistant Mono-Raise attempt class 1, "bottom up" methods:

// Starting with pairwise defeats, "make the case" for each candidate.
// How that is done depends on the variant, and returns a score per candidate.
// Highest score wins. For the descriptions below, assume WLOG that
// we're making the case for candidate A.

// RMR_DEFEATED:

// First set the defeat graph so that a candidate has an edge pointed at
// him if someone else beats him pairwise.

// Then do the following for k = 3...n:

// 		For every candidate B who is defeating A according to the graph,
//		check if B no longer disqualifies A on every set of cardinality k
//		or less. If so, flip the direction of the defeat B>A on the graph.

//		If A is now undefeated, return A's penalty as k. Otherwise, increase
//		the cardinality by one if possible, but if k=n, return A's penalty
//		as infinity (or n+1).

//	A's score is just his penalty, negated.

// RMR_DEFEATING:

// Find the highest k so that A ~(k)~> B for some other B, i.e. A
// disqualifies someone else over all sets of this cardinality. Break
// ties by counting the number of candidates who defeat A at the maximum k.
// This somehow doesn't pass Smith and the tiebreak is rather ugly, but
// the general idea is that raising A can never break A ~> B on any
// sub-election.

// RMR_TWO_WAY:
// Find the lowest rank at which A disqualifies someone and isn't
// disqualified by someone else. If there is none, then the rank is set to
// num candidates+1. The rank is A's penalty, so lower is better.

// RMR_SCHWARTZ_EXP:
// Starting at the second level, find the union of smallest sets of candidates
// who are undisqualified by everybody outside the set. Rearrange the current
// ordering of candidates so that members of these sets come before those who
// don't, while otherwise respecting the current order. Refine the order by
// going up the levels this way.
// Or alternatively: Starting at level k=2, calculate the Schwartz set based on
// disqualifications at that level. Update the base order to be (Schwartz at
// level k),(base order), then increment.
// This version passes Smith. Still not monotone, but it's much more decisive
// than the others here.

// --------------------

// The disqualification relation is the same as for the resistant/inner
// burial set.

// A lot of this is stolen from the resistant set code; I'll refactor
// later if this seems to show promise. TODO.

// Unfortunately, this method isn't monotone. But it's pushover-proof for
// some reason???

#include "../method.h"
#include "pairwise/matrix.h"
#include "../positional/simple_methods.h"

enum rmr_type { RMR_DEFEATED, RMR_DEFEATING, RMR_TWO_WAY, RMR_SCHWARTZ_EXP };

// beats[k][x][y] is true iff candidate x disqualifies y on
// every set of cardinality k and less.

typedef std::vector<std::vector<std::vector<bool> > > beats_tensor;

class rmr1 : public election_method {
	private:
		plurality plurality_method;
		rmr_type chosen_type;

		beats_tensor get_k_disqualifications(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates) const;

		condmat get_defeating_matrix(const beats_tensor & beats,
			const std::vector<bool> & hopefuls, int level) const;

		ordering iterative_schwartz(const beats_tensor & beats,
			const std::vector<bool> & hopefuls) const;

		int get_score_defeated(
			const beats_tensor & beats,
			const std::vector<bool> & hopefuls,
			int candidate, int num_candidates) const;

		int get_score_defeating(
			const beats_tensor & beats,
			const std::vector<bool> & hopefuls,
			int candidate, int num_candidates) const;

		int get_score_two_way(
			const beats_tensor & beats,
			const std::vector<bool> & hopefuls,
			int candidate, int num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			switch (chosen_type) {
				case RMR_DEFEATED:
					return "EXP: RMRA1-defeated";
				case RMR_DEFEATING:
					return "EXP: RMRA1-defeating-tiebreak";
				case RMR_TWO_WAY:
					return "EXP: RMRA1-two-way";
				case RMR_SCHWARTZ_EXP:
					return "EXP: RMRA1-Schwartz";
				default:
					throw std::invalid_argument("RMRA1: Invalid type!");
			}
		}

		// I'm not sure which is better; IFPP Method X uses
		// whole, and this uses fractional.
		rmr1(rmr_type chosen_type_in) : plurality_method(PT_FRACTIONAL) {
			if (chosen_type_in != RMR_DEFEATED &&
				chosen_type_in != RMR_DEFEATING &&
				chosen_type_in != RMR_TWO_WAY &&
				chosen_type_in != RMR_SCHWARTZ_EXP) {
				throw std::invalid_argument("RMRA1: Invalid type!");
			}

			chosen_type = chosen_type_in;
		}
};