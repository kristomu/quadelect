#pragma once

#include "singlewinner/positional/simple_methods.h"
#include "pairwise/matrix.h"

// A subelection, given by a set S, is an election with every candidate not
// in S eliminated. This class counts the first preferences for every
// subelection of a given election. The first preference information is used
// in resistant set-based methods to determine or elect from the resistant set.

// beats[k][x][y] is true iff candidate x disqualifies y on
// every set of cardinality k and less.

typedef std::vector<std::vector<std::vector<bool> > > disqual_tensor;

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

		// Determine the disqualification relation for each level
		// of set cardinality. disqualifies[i][a][b] gives whether
		// a ~> b when considering subelections of i members or
		// fewer (cumulative) or subelections of exactly i members
		// (not cumulative).
		disqual_tensor get_level_disqualifications(
			const std::vector<bool> & hopefuls,
			bool cumulative) const;

		// I'm not sure which is better; IFPP Method X uses
		// whole, and this uses fractional.
		subelections() : plurality_method(PT_FRACTIONAL) {}
};

class subelection_tools {
	public:
		// Turn a disqualification level into a boolean
		// Condorcet matrix.
		static condmat get_defeating_matrix(const disqual_tensor & beats,
			const std::vector<bool> & hopefuls, size_t level);
};