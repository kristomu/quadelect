#pragma once

// Not centroid. Geometric median.

// Brute force solution to the multiwinner Kemeny idea

// Multiwinner Kemeny: find the ranks that constitute the centroid ranks,
// subject to that each candidate can be mentioned in each rank position
// (column) no more than once.

// Ranks are struct vectors of ints, easier that way.

#include "auxiliary/greedy_cluster.h"
#include "kemeny_solvers/assignment.h"
#include "multiwinner/methods/methods.h"
#include "tools/ballot_tools.h"

#include "auxiliary/qballot.h"

#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

class fc_kemeny : public multiwinner_method {

	private:
		// QnD
		std::vector<std::vector<bool> > tournament_matrix(const
			std::vector<int> & rank, int maxcand) const;
		// Kemeny distance
		int p_distance(const std::vector<std::vector<bool> > & cmat_a,
			const std::vector<std::vector<bool> > & cmat_b,
			int maxcand) const;
		int p_distance(const std::vector<int> & a,
			const std::vector<int> & b, int maxcand) const;

		int tiebreak(const std::vector<std::vector<int> > & a, int maxcand) const;
		void permutation(int input, std::vector<int> & rank) const;
		void print_rank(const std::vector<int> & rank) const;
		pair<int, int> distance_to_closest(const std::vector<int> &
			our_rank, const std::vector<std::vector<int> > &
			centroids, int numcands) const;
		int get_continuous_score(const std::vector<q_ballot> & ballots,
			const std::vector<std::vector<int> > & centroids,
			int numcands, assignment & solver,
			bool brute_calc) const;
		int get_score(const std::vector<q_ballot> & ballots,
			const std::vector<std::vector<int> > & centroids,
			int numcands) const;
		bool verify(const std::vector<std::vector<int> > & centroids,
			int numcands) const;
		q_ballot build_ballot(int strength, string order) const;
		int factorial(int n) const;
		void recurse_ranking(const std::vector<q_ballot> & ballots,
			const std::vector<int> & arch,
			std::vector<std::vector<int> > & recordholder,
			double & record,
			std::vector<std::vector<int> > & centroids,
			int pos, int offset,
			int numcands, int fact,
			assignment & solver,
			const std::vector<std::vector<std::vector<bool> > > &
			voter_matrices,
			std::vector<std::vector<std::vector<bool> > > &
			medoid_matrices) const;

		bool use_continuous;

	public:
		council_t get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		fc_kemeny(bool continuous_in) {
			use_continuous = continuous_in;
		}

		string name() const {
			if (use_continuous) {
				return ("CFC-Kemeny (EXP)");
			} else {
				return ("FC-Kemeny (EXP)");
			}
		}
};