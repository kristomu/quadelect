#pragma once

// Not centroid. Geometric median.

// Brute force solution to the multiwinner Kemeny idea

// Multiwinner Kemeny: find the ranks that constitute the centroid ranks,
// subject to that each candidate can be mentioned in each rank position
// (column) no more than once.

// Ranks are struct vectors of ints, easier that way.

#include "auxiliary/greedy_cluster.h"
#include "auxiliary/assignment.h"
#include "multiwinner/methods.h"
#include "tools/ballot_tools.h"

#include "auxiliary/qballot.h"

#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

using namespace std;

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
		std::list<size_t> get_council(size_t council_size, size_t num_candidates,
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

std::vector<std::vector<bool> > fc_kemeny::tournament_matrix(
	const std::vector<int> & rank, int maxcand) const {

	std::vector<std::vector<bool> > toRet(maxcand,std::vector<bool>(maxcand,
			false));

	for (size_t counter = 0; counter < rank.size(); ++counter)
		for (size_t sec = counter+1; sec < rank.size(); ++sec) {
			toRet[rank[counter]][rank[sec]] = true;
		}

	return (toRet);
}

int fc_kemeny::p_distance(const std::vector<std::vector<bool> > & cmat_a,
	const std::vector<std::vector<bool> > & cmat_b,
	int maxcand) const {

	int mismatch_count = 0;

	for (int counter = 0; counter < maxcand; ++counter)
		for (int sec = counter+1; sec < maxcand; ++sec)
			if (cmat_a[counter][sec] ^ cmat_b[counter][sec]) {
				++mismatch_count;
			}

	return (mismatch_count);
}

int fc_kemeny::p_distance(const std::vector<int> & a,
	const std::vector<int> & b,
	int maxcand) const {

	std::vector<std::vector<bool> > cmat_a = tournament_matrix(a, maxcand),
									cmat_b = tournament_matrix(b, maxcand);

	return (p_distance(cmat_a, cmat_b, maxcand));
}


// Improvised tiebreak: returns sum distance between centroids (all pairs).
// Is greater better, or less better? We'll try less for now.
int fc_kemeny::tiebreak(const std::vector<std::vector<int> > & a,
	int maxcand) const {

	int sum = 0;

	for (size_t counter = 0; counter < a.size(); ++counter)
		for (size_t sec = counter+1; sec < a.size(); ++sec) {
			sum += p_distance(a[counter], a[sec], maxcand);
		}

	return (sum);
}

void fc_kemeny::permutation(int input, std::vector<int> & rank) const {
	for (size_t counter = 0; counter < rank.size(); ++counter) {
		size_t radix = rank.size() - counter;
		size_t s = input % radix;
		input = (input - s)/radix;

		swap(rank[counter], rank[counter + s]);
	}
}

void fc_kemeny::print_rank(const std::vector<int> & rank) const {

	for (size_t counter = 0; counter < rank.size(); ++counter) {
		if (counter > 0) {
			cout << ">";
		}
		cout << (char)(rank[counter] + 'A');
	}
	//cout << endl;
}

// Returns pair where the first is the record distance and the second is
// the recordholder.
pair<int, int> fc_kemeny::distance_to_closest(const std::vector<int> &
	our_rank,
	const std::vector<std::vector<int> > & centroids, int numcands) const {

	pair<int, int> record(-1, -1);

	for (size_t counter = 0; counter < centroids.size(); ++counter) {
		int candidate = p_distance(our_rank, centroids[counter],
				numcands);

		if (record.first == -1 || record.first > candidate) {
			record.first = candidate;
			record.second = counter;
		}
	}

	return (record);
}

// Behold the joy of linear programming!
int fc_kemeny::get_continuous_score(const std::vector<q_ballot> & ballots,
	const std::vector<std::vector<int> > & medoids, int numcands,
	assignment & solver, bool brute_calc) const {

	// Do gradually later.

	for (size_t counter = 0; counter < ballots.size() && brute_calc; ++counter)
		for (size_t sec = 0; sec < medoids.size(); ++sec)
			solver.set_constraint(counter, sec,
				p_distance(ballots[counter].rank,
					medoids[sec], numcands));

	// TODO: Implement last record so that if infeasible, it can abort
	// quickly.

	double score = solver.calc_minimum();

	assert(solver.success());

	return (score);
}

int fc_kemeny::get_score(const std::vector<q_ballot> & ballots,
	const std::vector<std::vector<int> > & medoids, int numcands) const {

	// Force a clustering of equal size. For ideas, see the original
	// fc_kemeny.cc - linearization might be possible!

	std::vector<std::vector<int> > scores;

	size_t counter, sec;

	for (counter = 0; counter < ballots.size(); ++counter) {
		std::vector<int> cur(medoids.size(), 0);

		for (sec = 0; sec < medoids.size(); ++sec)
			cur[sec] = p_distance(ballots[counter].rank,
					medoids[sec], numcands);

		for (sec = 0; sec < (size_t)ballots[counter].strength; ++sec) {
			scores.push_back(cur);
		}
	}

	return (get_clustering_error(scores, medoids.size()));
}

bool fc_kemeny::verify(const std::vector<std::vector<int> > & centroids,
	int numcands) const {
	// No candidate can be mentioned more than once in a single rank.
	// Maybe this is unneccessary and the best centroids will have this
	// property anyway.

	for (int column = 0; column < 1;
		++column) { //centroids[0].size(); ++column) {
		std::vector<bool> seen(numcands, false);
		for (size_t sec = 0; sec < centroids.size(); ++sec)
			if (seen[centroids[sec][column]]) {
				return (false);
			} else	{
				seen[centroids[sec][column]] = true;
			}
	}

	return (true); // for now
}

q_ballot fc_kemeny::build_ballot(int strength, string order) const {

	q_ballot toret;
	toret.strength = strength;

	for (size_t counter = 0; counter < order.size(); ++counter) {
		toret.rank.push_back(order[counter] - 'A');
	}

	return (toret);
}

int fc_kemeny::factorial(int n) const {
	if (n <= 1) {
		return (n);
	}
	return (n * factorial(n-1));
}

// This can be sped up by having the permutation function return in
// sorted order. Then we can do something like "if first is A, start next at
// B first". PROOF OF CONCEPT!

// Centroids is the vector of ranks that constitute our centroid (really) set.
// Pos is the position (determining first member, second member), numerical
// ranking is used so we don't check "B, A" if we've already checked "A, B"
void fc_kemeny::recurse_ranking(const std::vector<q_ballot> & ballots,
	const std::vector<int> & arch,
	std::vector<std::vector<int> > & recordholder, double & record,
	std::vector<std::vector<int> > & centroids, int pos,
	int offset, int numcands, int fact, assignment & solver,
	const std::vector<std::vector<std::vector<bool> > > & voter_matrices,
	std::vector<std::vector<std::vector<bool> > > & medoid_matrices) const {

	int counter;
	int maximum = centroids.size();

	if (pos == maximum) {
		// REQUIRED for number ten!
		// Why? Find out.
		if (!verify(centroids, numcands)) {
			return;
		}

		double cand;
		if (use_continuous)
			cand = get_continuous_score(ballots, centroids,
					numcands, solver, false);
		else {
			cand = get_score(ballots, centroids, numcands);
		}

		bool replace_with_new = cand < record || record == -1;
		// Run improvised tiebreak. Whoever has the least distance
		// between centroids wins (or should this be greatest?)
		// (Must be > to pass the DPC?)
		// We might have to have a tiebreak inside LP too. Or maybe
		// not?
		if (!replace_with_new && cand == record)
			replace_with_new = (tiebreak(centroids, numcands) >
					tiebreak(recordholder, numcands));

		// Beware ties
		if (replace_with_new) {
			record = cand;
			recordholder = centroids;
		}

		return;
	}

	if (offset == numcands) {
		return;
	}
	if (numcands - offset < maximum - pos) {
		return;    // not enough space
	}

	// Recurse!
	// See other example for why we do it like this
	recurse_ranking(ballots, arch, recordholder, record, centroids, pos,
		offset+1, numcands, fact, solver, voter_matrices,
		medoid_matrices);

	for (counter = offset; counter < fact; ++counter) {
		centroids[pos] = arch;
		permutation(counter, centroids[pos]);

		medoid_matrices[pos] = tournament_matrix(centroids[pos],
				numcands);

		if (use_continuous)
			for (size_t ball = 0; ball < ballots.size(); ++ball)
				solver.set_constraint(ball, pos,
					p_distance(voter_matrices[ball],
						medoid_matrices[pos],
						numcands));

		recurse_ranking(ballots, arch, recordholder, record,
			centroids, pos+1, offset+1, numcands, fact,
			solver, voter_matrices, medoid_matrices);
	}
}


// Make properly recursive later

std::list<size_t> fc_kemeny::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & vballots) const {

	election_t compressed_ballots = ballot_tools().compress(vballots);

	std::vector<int> rank(num_candidates, 0);
	iota(rank.begin(), rank.end(), 0);

	assert(num_candidates < 12);

	std::vector<q_ballot> xlat_ballots;
	std::vector<std::vector<std::vector<bool> > > ballot_matrices;
	// translate here
	// Doesn't handle equal rank, etc.
	double num_voters = 0;
	for (election_t::const_iterator pos = compressed_ballots.begin();
		pos != compressed_ballots.end(); ++pos) {
		q_ballot next_one;
		num_voters += pos->get_weight();
		next_one.strength = pos->get_weight();
		for (ordering::const_iterator spos = pos->contents.begin();
			spos != pos->contents.end(); ++spos) {
			next_one.rank.push_back(spos->get_candidate_num());
		}
		xlat_ballots.push_back(next_one);
		ballot_matrices.push_back(tournament_matrix(next_one.rank,
				num_candidates));
	}

	double record = -1;
	std::vector<std::vector<int> > recordholder(council_size,
		std::vector<int>(num_candidates, -1));
	std::vector<std::vector<int> > centroids(council_size);

	assignment check(num_voters, xlat_ballots.size(), council_size,
		xlat_ballots);

	std::vector<std::vector<std::vector<bool> > > medoid_matrices(
		council_size);

	recurse_ranking(xlat_ballots, rank, recordholder, record, centroids, 0,
		0, num_candidates, factorial(num_candidates), check,
		ballot_matrices, medoid_matrices);

	std::list<size_t> toRet;

	std::vector<int> already(num_candidates, false);

	for (size_t counter = 0; counter < recordholder.size(); ++counter) {
		toRet.push_back(recordholder[counter][0]);
		assert(!already[recordholder[counter][0]]);
		already[recordholder[counter][0]] = true;
	}

	return (toRet);
}
