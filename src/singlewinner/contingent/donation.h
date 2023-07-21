// The contingent vote "with donation". This is an experimental
// method that tries to reduce the monotonicity failures of the
// contingent vote with the following observation:

// If A is the Plurality winner and C is the other contestant, and
// we're in an A>B>C>A Condorcet cycle, then A might have preferred
// that some of his voters had voted B instead, so that B's first
// preferences would exceed C's, which would have got A paired up with
// B and hence made A win.

// The get_possible_finalist_score method checks if it's possible for
// A (the finalist) to transfer some of his first preferences to some
// other candidate C so that A (after donations/transfers) does not fall
// below B's Plurality count, but C now exceeds B's. If so, then A could
// make the final be against C instead, so the function returns the
// maximum of A>B and A>C.

#pragma once

#include "contingent.h"

class donated_contingent_vote : public contingent_vote {
	protected:
		// Returns the finalist score of the candidate "finalist".
		virtual double get_possible_finalist_score(size_t finalist,
			size_t challenger, const condmat & pairwise_matrix,
			const std::vector<bool> & hopefuls,
			const std::vector<double> & plur_scores) const;

	public:
		virtual std::string name() const {
			return "EXP: Contingent vote + donation";
		}
};