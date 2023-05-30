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

// We extend the method so that it produces an ordering by first checking
// which candidates can win by usual DSC rules if we break ties on equal-
// support coalitions in favor of that candidate. We consider all candidates
// for which that is true to have first rank, exclude them from the initial
// set of candidates (before any intersections are done), and repeat the
// process to find which candidates have second rank, and so on until we've
// determined the rank of every candidate.

// This is rather slow but is a more conservative approach and should work
// to generalize DSC and provide a social ordering (albeit not a measure of
// support for each candidate).

// We strictly speaking need a proof that any candidate X that can win with
// some tiebreaker for coalitions of the same support, can win if the
// tiebreaker consistently favors X, sorting coalitions containing X before
// coalitions not containing X, with further order arbitrary.

// WOODALL, Douglas R. Monotonicity of single-seat preferential election rules.
// Discrete Applied Mathematics, 1997, 77.1: 81-98.


// Test vector (to implement later?):

// 16: A > B > C
// 29: A > C > B
// 20: B > A > C
// 18: B > C > A
// 17: C > A > B
// 27: C > B > A

// Should return A = C > B

// TODO: Make test vector for my EM counterexample to the obvious algorithm
// that used to be here, with coalitions like

// 100: ABC
// 80: BC
// 20: A
// 19: B

// which should have B win, not A.

#ifndef _VOTE_DSC
#define _VOTE_DSC

#include "desc_coalition.h"

class dsc : public desc_coalition_method {

	protected:
		std::vector<coalition_entry> get_coalitions(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates) const;

	public:
		std::string name() const {
			return ("DSC");
		}
};

#endif