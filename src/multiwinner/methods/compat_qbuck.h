#pragma once

#include <list>
#include "methods.h"

// QLTD-PR as copied from secprelect (where it gets extremely good scores).

class old_qltd_pr : public multiwinner_method {
	private:
		std::vector<std::vector<int> > construct_old_ballots(
			const election_t & ballots, size_t num_candidates) const;

		// BEWARE: Pointers!
		std::vector<int> QuotaBucklin(
			const election_t & ballots,
			const std::vector<std::vector<int> > & rankings,
			size_t num_candidates, size_t council_size, bool hare,
			bool use_card, const std::vector<std::vector<double> > *
			rated_ballots) const;

	public:
		council_t get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("Compat-QLTD-PR");
		}

};
