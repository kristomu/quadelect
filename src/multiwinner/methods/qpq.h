#pragma once

#include "methods.h"
#include <vector>
#include <list>

// KNOWN BUG: Does not handle negative scores well. I have no idea why,
// since this is a ranked method. Use multiwinner_spatial with Euclidean
// distance and no noise to reproduce.

class QPQ : public multiwinner_method {
	private:
		ordering::const_iterator ballot_contribution(
			const std::vector<bool> & eliminated,
			const std::vector<bool> & elected,
			const ballot_group & ballot) const;

		council_t get_council(std::vector<bool> & eliminated,
			size_t council_size, size_t num_candidates,
			int num_voters,
			const election_t & ballots,
			bool restart_on_elimination) const;

		bool recursive;
		double C_val;	// 1 = D'Hondt, 0.5 = Sainte-Lague,
		// EXPERIMENTAL: -1 = WDS dynamic

		double getC(size_t council_size, size_t num_candidates) const;

	public:
		council_t get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		std::string name() const;

		QPQ(double c_in, bool recurs_in) {
			C_val = c_in;
			recursive = recurs_in;
		}
};