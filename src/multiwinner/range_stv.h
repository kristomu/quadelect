// A Range variant of STV. It goes like this:
// While the council has empty seats:
// 	1. Elect whoever has the greatest score and is not already elected.
// 	2. Reweight every ballot by max(0, (N-D(rN/R))/N)
// 		where N is number of voters, D is the Droop quota, r is
// 		that ballot's Range score for the winner, and R is the
// 		electorate's total (first round) score for the winner.

#pragma once

#include "methods.h"
#include <list>

class LRangeSTV : public multiwinner_method {
	private:
		std::vector<double> count_score(size_t num_candidates,
			const election_t & ballots,
			const std::list<double> & weights) const;

		ordering get_possible_winners(size_t num_candidates,
			const election_t & ballots,
			const std::list<double> & weights) const;

		int elect_next(size_t council_size, size_t num_candidates,
			std::vector<bool> & elected,
			election_t & altered_ballots) const;

	public:
		std::list<size_t> get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("?Linear Range STV");
		}
};
