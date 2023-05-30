// Young's method. This is not Kemeny-Young, but rather a method that scores
// every candidate according to how large a subset of the voters we can form
// while still having the candidate we want be a CW. It's NP-complete and hard
// to approximate because a suboptimal solution may fail to be a solution
// at all: removing some ballots from a solution may lead the candidate to no
// longer be the CW.

// We solve it through integer programming. The integer program is

// c*'s Young score:
//	max sum over values x[i] so that
//		for all candidates a except for c*,
//			sum over i
//				 x[i] * e[i][c*][a]
//                        > 0
// for all i, x[i] is integer, 0 >= x[i] >= wt[i], where wt[i] is the weight of
// the ith ballot,

// where e[i][c*][a] is 1 if the ith voter ranks c* above a, otherwise -1. This
// works because if c* is a CW, for any other a, more voters must be voting
// c* above a (e[i][c*][a] = 1) than below a (e[i][c*][a] = -1), thus
// e[i][c*][a] > 0.

// If the candidates are equally ranked, e[i][c*][a] = 0; or we could employ a
// variant of symmetric completion and have e[i][c*][a] = e[i][a][c*] = 0.5.
// It is not actual symmetric completion because that would require additional
// ballots since Young is per-ballot, not per-CM.

// The integer programming version restricts ballot weights to integers. A
// relaxed, linear programming version does not.

#ifndef _VOTE_YOUNG
#define _VOTE_YOUNG

#include "../pairwise/matrix.h"
#include "method.h"

#include <iterator>
#include <iostream>

#include <glpk.h>
#include <assert.h>


class young : public election_method {
	private:
		std::string cached_name;
		bool is_sym_comp, is_relaxed;

		// returns (-1, -1) on error. The first of the pair is the
		// linear programming score. The second is the integer
		// programming score, or -1 if running relaxed.
		std::pair<double, double> get_young_score(
			const std::list<ballot_group> & papers,
			size_t candidate, size_t num_candidates,
			size_t num_ballots, const std::vector<bool> & hopefuls,
			bool relaxed, bool symmetric_completion,
			bool debug) const;

		std::string determine_name() const;

	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			return (cached_name);
		}

		young(bool use_sym_comp, bool use_relax);

};

#endif
