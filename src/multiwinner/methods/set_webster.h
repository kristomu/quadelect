#pragma once

// The Set Webster method works like this:
//	Find the largest value x so that, when solid coalitions' support values are
//	transformed by y = round(support * x), there exists at least one council
//	assignment of the desired size that passes the criteria
//		"For every solid coalition S, at least min(|S|, y(S)) members from this
//		 coalition must be elected."

// Since I don't know an algorithm to determine if such a council exists, I just
// try every possible set inside a bisection search that finds the greatest value
// of x.

// Note, this may be different from the earlier (thirdelect etc) versions of
// Set Webster, where I seem to have been trying to make somethind that reduces
// to DAC or DSC in the single winner case.

#include "coalitions/coalitions.h"

#include "exhaustive/set_webster_helper.h"

class set_webster : public multiwinner_method {
	private:
		exhaustive_optima get_opt(double proposed_factor, size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

	public:
		council_t get_council(size_t council_size,
			size_t num_candidates, const election_t & ballots) const;

		std::string name() const {
			return "Set Webster";
		}
};
