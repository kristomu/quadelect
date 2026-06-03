#pragma once

// Not centroid. Geometric median.

// Brute force solution to the multiwinner Kemeny idea

// Multiwinner Kemeny: find the ranks that constitute the centroid ranks,
// subject to that each candidate can be mentioned in each rank position
// (column) no more than once.

// Ranks are struct vectors of ints, easier that way.

#include "multiwinner/methods/methods.h"
#include "tools/ballot_tools.h"
#include "auxiliary/qballot.h"

#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

class mw_kemeny : public multiwinner_method {

	private:
		// QnD
		ballot_tools btools;

		std::vector<std::vector<bool> > tournament_matrix(const
			std::vector<int> & rank, size_t maxcand) const;
		int p_distance(const std::vector<int> & a,
			const std::vector<int> & b, size_t maxcand) const;
		int tiebreak(const std::vector<std::vector<int> > & a,
			size_t maxcand) const;
		void permutation(int input, std::vector<int> & rank) const;
		void print_rank(const std::vector<int> & rank) const;
		std::pair<int, int> distance_to_closest(
			const std::vector<int> & our_rank,
			const std::vector<std::vector<int> > &
			centroids, int numcands) const;
		int get_score(const std::vector<q_ballot> & ballots,
			const std::vector<std::vector<int> > & centroids,
			int numcands) const;
		bool verify(const std::vector<std::vector<int> > & centroids,
			int numcands) const;
		q_ballot build_ballot(int strength, std::string order) const;
		int factorial(int n) const;
		void recurse_ranking(const std::vector<q_ballot> & ballots,
			const std::vector<int> & arch,
			std::vector<std::vector<int> > & recordholder,
			int & record, std::vector<std::vector<int> > & centroids,
			int pos, int offset,
			int numcands, int fact) const;

	public:
		council_t get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("MW-Kemeny (EXP)");
		}
};