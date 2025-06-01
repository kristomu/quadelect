#pragma once

// The "inner burial set" defined by the criterion of
// http://lists.electorama.com/pipermail/election-methods-electorama.com/2023-August/004730.html

// If no matter what candidates you eliminate, if you don't eliminate A and B,
// and A has more than 1/(number of remaining candidates) of the total support,
// and A beats B pairwise, then B is outside this set.

// Though brittle, this may be useful for burial resistance analysis. If nothing
// else, it will be useful for checking if methods pass the generalized DMTCBR
// criterion.

#include "../method.h"
#include "pairwise/matrix.h"
#include "../positional/simple_methods.h"

// Helper class for first preference counts for every subelection (way to
// eliminate candidates).

class subelections {
	private:
		plurality plurality_method;

	public:
		// The hopeful power set lists the candidates included in each
		// subelection, while the first_pref_scores list their first
		// preferences, and the num_remaining_candidates and num_remaining
		// voters vectors list how many candidates are included in the
		// subelection and how many continuing (non-exhausted) voters are
		// involved.

		std::vector<std::vector<bool > > hopeful_power_set;
		std::vector<std::vector<double> > first_pref_scores;
		std::vector<int> num_remaining_candidates;
		std::vector<double> num_remaining_voters;

		// The hopefuls that make up the full election.
		std::vector<bool> included_candidates;
		size_t num_candidates; // including already eliminated (non-hopefuls)

		// Determine the subelection first-preference counts for
		// the given election. If tiebreak is true, add a small
		// value to lower-numbered candidates so that true ties
		// never occur. (Doing so breaks neutrality.)
		void count_subelections(const election_t & papers,
			const std::vector<bool> & hopefuls,
			bool tiebreak);

		// I'm not sure which is better; IFPP Method X uses
		// whole, and this uses fractional.
		subelections() : plurality_method(PT_FRACTIONAL) {}
};

class inner_burial_set : public election_method {
	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return "Inner burial set";
		}
};