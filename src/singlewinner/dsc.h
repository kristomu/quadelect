// Woodall's Descending Solid Coalitions method. 

// Every possible set of candidates is given a score equal to the number of 
// voters who are solidly committed to the candidates in that set. 

// A voter is solidly committed to a set of candidates if he ranks every 
// candidate in this set strictly above every candidate not in the set.

// The sets are then considered in turn, from those with the greatest score to 
// those with the least. When a set is considered, every candidate not in the 
// set becomes ineligible to win, unless this would cause all candidates to be 
// ineligible, in which case that set is ignored.

// (Definition from Electowiki)

// We extend the method so that it produces a score for each candidate instead
// of just selecting a winner. For each candidate X, we keep a count of the
// number of voters, going down the coalition list and intersecting as before
// unless the intersection would produce an empty set *or eliminate X*.
// X's score is the support of the set that first led to every candidate but
// X being eliminated.

// For each such candidate, break ties in favor of sets that contain the
// candidate. From the perspective of candidate X, each set of coalitions
// of the same strength/support can be divided into two: the ones that 
// contain X and the ones that do not. If we first go through all the ones
// that contain X, then if there exists a way to intersect away every
// candidate but X, then X will be left as the sole candidate once we're done.
// Otherwise, X won't be, but as many non-X candidates as is possible will 
// have been removed once we're done, and the method will skip all the sets
// that do not contain X afterwards until it gets to the next same-support
// set of coalitions (with lower support). So the order within the 
// "contains X" half doesn't matter, and the order within the "doesn't 
// contain X" half doesn't, either.
// We thus get a method that is neutral without having to employ random
// tiebreakers.

// WOODALL, Douglas R. Monotonicity of single-seat preferential election rules.
// Discrete Applied Mathematics, 1997, 77.1: 81-98.


// Test vector (to implement later?):

// 16: A > B > C
// 29: A > C > B 
// 20: B > A > C 
// 18: B > C > A 
// 17: C > A > B 
// 27: C > B > A 

// Should return A = C > B with scores A: 45, B: 38, C: 45

#ifndef _VOTE_DSC
#define _VOTE_DSC

#include "method.h"

// Helper class. X is greater than Y if X's score is greater, or if it's
// equal and X contains the priority candidate whereas Y does not.

class coalition_entry {
	public:
		std::set<int> coalition;
		double score;
		int priority_candidate;

		bool operator>(const coalition_entry & other) const {
			if (score != other.score)
				return (score > other.score);

			return (coalition.find(priority_candidate) != coalition.end() && 
					other.coalition.find(priority_candidate) == 
					other.coalition.end());
		}

		coalition_entry(double score_in, const std::set<int> & coalition_in) {
			coalition = coalition_in;
			score = score_in;
			priority_candidate = -1;
		}
};

class dsc : public election_method {

	private:
		void sort_by_candidate(
			std::vector<coalition_entry> & coalitions, 
			int candidate) const;
		double get_candidate_score(
			std::vector<coalition_entry> & coalitions, 
			int candidate, int num_candidates) const;

	public:
		std::pair<ordering, bool> elect_inner(
				const std::list<ballot_group> & papers,
				const std::vector<bool> & hopefuls,
				int num_candidates, cache_map * cache,
				bool winner_only) const;

		string name() const { return ("DSC"); }
};

#endif