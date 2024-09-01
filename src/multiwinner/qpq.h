#pragma once

#include "methods.h"
//#include "../tiebreaker.cc"
#include <vector>
#include <list>

class QPQ : public multiwinner_method {
	private:
		ordering::const_iterator ballot_contribution(
			const std::vector<bool> & eliminated,
			const std::vector<bool> & elected,
			const ballot_group & ballot) const;

		std::list<int> get_council(std::vector<bool> & eliminated,
			int council_size, int num_candidates,
			int num_voters,
			const election_t & ballots,
			bool restart_on_elimination) const;

		bool recursive;
		double C_val;	// 1 = D'Hondt, 0.5 = Sainte-Lague,
		// EXPERIMENTAL: -1 = WDS dynamic

		double getC(int council_size, int num_candidates) const;

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		std::string name() const;

		QPQ(double c_in, bool recurs_in) {
			C_val = c_in;
			recursive = recurs_in;
		}
};