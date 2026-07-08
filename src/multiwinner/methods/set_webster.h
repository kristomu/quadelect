#pragma once

// The Set Webster method works like this:
//	Find the largest value x so that, when solid coalitions' support values are
//	transformed by y = round(support * x), there exists at least one council
//	assignment of the desired size that passes the criteria
//		"For every solid coalition S, at least min(|S|, y(S)) members from this
//		 coalition must be elected."

// This can be done in a brute-force manner by first calculating
// every solid coalition, and then for each proposed council, doing
// an intersection of maxima to find the largest x that passes the
// criterion.

// I don't know if there exists a better algorithm than this;
// there might even be a reduction from some set cover-like problem.

// Note, this may be different from the earlier (thirdelect etc) versions of
// Set Webster, where I seem to have been trying to make somethind that reduces
// to DAC or DSC in the single winner case.

#include "coalitions/coalitions.h"
#include "methods.h"

#include "exhaustive/method.h" // for combo. Fix later.

class set_webster : public multiwinner_method {
	private:
		// Returns the least value that is not a valid factor
		// for the proposed outcome. (We do it like this because
		// the max *permissible* factor is an epsilon less than
		// this, i.e. the allowed interval may be open.)
		double get_min_disallowed_factor(
			combo::it outcome_start, combo::it outcome_end,
			const std::vector<coalition_data> &
			solid_coalitions) const;

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return "Set Webster";
		}
};
