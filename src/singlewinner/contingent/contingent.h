// The contingent vote. First we use first preferences to determine
// the top two. Suppose they're A and B. Then A's score is the
// pairwise magnitude A>B while B's score is B>A and everybody else's
// score is zero.

// Ties are handled as follows: Every possible finalist pair (A,B)
// is associated with a score A>B to A, and B>A to B. A pair of
// candidates can be finalists if:
//		- A and B are both tied for first
//	or	- there's only one winner (A), and B is one of the second-
//			placers.

// Each candidate gets a final score that's the maximum possible out of
// the possible finalist pairs. This is slightly more burial-resistant
// than the "obvious" way of giving each candidate its mean finalist
// score.

#pragma once

#include "../method.h"

class contingent_vote : public election_method {
	protected:
		// Returns the finalist score of the candidate "finalist".
		virtual double get_possible_finalist_score(size_t finalist,
			size_t challenger, const condmat & pairwise_matrix,
			const std::vector<bool> & hopefuls,
			const std::vector<double> & plur_scores) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const std::list<ballot_group> & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "Contingent vote";
		}
};