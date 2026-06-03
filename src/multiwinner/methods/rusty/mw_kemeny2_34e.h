#pragma once

// Not centroid. Geometric median.

// Brute force solution to the multiwinner Kemeny idea

// Multiwinner Kemeny: find the ranks that constitute the centroid ranks,
// subject to that each candidate can be mentioned in each rank position
// (column) no more than once.

// Ranks are struct vectors of ints, easier that way.

#include "multiwinner/methods/methods.h"
#include "tools/ballot_tools.h"

#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

#include "auxiliary/qballot.h"

using namespace std;

class mw_kemeny2_34e : public multiwinner_method {

	private:
		ballot_tools btools;

		// QnD
		std::vector<std::vector<bool> > tournament_matrix(const
			std::vector<int> & rank, int maxcand) const;
		int p_distance(const std::vector<int> & a,
			const std::vector<int> & b, int maxcand) const;
		int tiebreak(const std::vector<std::vector<int> > & a, int maxcand) const;
		void permutation(int input, std::vector<int> & rank) const;
		void print_rank(const std::vector<int> & rank) const;
		pair<int, int> distance_to_closest(const std::vector<int> &
			our_rank, const std::vector<std::vector<int> > &
			centroids, int numcands) const;
		int get_score(const std::vector<q_ballot> & ballots,
			const std::vector<std::vector<int> > & centroids,
			std::vector<int> & support,
			int numcands) const;
		pair<bool, council_t > verify(
			const std::vector<std::vector<int> > & centroids,
			const std::vector<int> & support, int numcands,
			bool fill_list) const;
		q_ballot build_ballot(int strength, string order) const;
		int factorial(int n) const;
		void recurse_ranking(const std::vector<q_ballot> & ballots,
			const std::vector<int> & arch,
			std::vector<std::vector<int> > & recordholder,
			int & record, std::vector<std::vector<int> > & centroids,
			int pos, int lastnum,
			int numcands, int fact) const;

	public:
		council_t get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		string name() const {
			return ("SL-Kemeny (EXP, 34e)");
		}
};
