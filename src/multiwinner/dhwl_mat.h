// D'Hondt without lists.

// NOTE: The deweighting calculation might be very wrong. In
// http://lists.electorama.com/pipermail/election-methods-electorama.com/2008-October/121082.html
// I said that if every preference below the first elected candidate
// should be deweighted by (e.g. for Sainte-Laguë) 1/3, every preference
// below the first two by 1/3 * 1/5, etc.
// arguably the proper order is to deweight the first by 1/3, the second by
// 1/5, and so on. TODO: Implement that later.

#pragma once

#include "pairwise/matrix.h"
#include <vector>

class DHwLmatrix : public condmat {
	private:
		std::vector<bool> elected;
		double C;

		double deweight(int num_higher_elected_prefs) const;

		void count_ballots(const election_t & scores,
			int num_candidates);

		void add(size_t candidate, size_t against,
			double value);

	public:
		void set_elected(const std::vector<bool> & source);
		void set_elected(int elected_idx);

		DHwLmatrix(const election_t & scores,
			const std::vector<bool> & already_elected,
			int num_candidates, pairwise_type type_in,
			bool tie_at_top, double C_in);
};