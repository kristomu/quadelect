#pragma once

// Not centroid. Geometric median.

// Brute force solution to the multiwinner Kemeny idea

// Multiwinner Kemeny: find the ranks that constitute the centroid ranks,
// subject to that each candidate can be mentioned in each rank position
// (column) no more than once.

// Ranks are struct vectors of ints, easier that way.

#include "multiwinner/methods.h"
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
		pair<bool, std::list<size_t> > verify(
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
		std::list<size_t> get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		string name() const {
			return ("SL-Kemeny (EXP, 34e)");
		}
};

std::vector<std::vector<bool> > mw_kemeny2_34e::tournament_matrix(
	const std::vector<int> & rank,
	int maxcand) const {

	std::vector<std::vector<bool> > toRet(maxcand,std::vector<bool>(maxcand,
			false));

	for (size_t counter = 0; counter < rank.size(); ++counter)
		for (size_t sec = counter+1; sec < rank.size(); ++sec) {
			toRet[rank[counter]][rank[sec]] = true;
		}

	return (toRet);
}

int mw_kemeny2_34e::p_distance(const std::vector<int> & a,
	const std::vector<int> & b,
	int maxcand) const {

	std::vector<std::vector<bool> > cmat_a = tournament_matrix(a, maxcand),
									cmat_b = tournament_matrix(b, maxcand);

	int mismatch_count = 0;

	for (int counter = 0; counter < maxcand; ++counter)
		for (int sec = counter+1; sec < maxcand; ++sec)
			if (cmat_a[counter][sec] ^ cmat_b[counter][sec]) {
				++mismatch_count;
			}

	return (mismatch_count);
}

// Improvised tiebreak: returns sum distance between centroids (all pairs).
// Is greater better, or less better? We'll try less for now.
int mw_kemeny2_34e::tiebreak(const std::vector<std::vector<int> > & a,
	int maxcand) const {

	int sum = 0;

	for (size_t counter = 0; counter < a.size(); ++counter)
		for (size_t sec = counter+1; sec < a.size(); ++sec) {
			sum += p_distance(a[counter], a[sec], maxcand);
		}

	return (sum);
}

void mw_kemeny2_34e::permutation(int input,
	std::vector<int> & rank) const {
	for (size_t counter = 0; counter < rank.size(); ++counter) {
		int radix = rank.size() - counter;
		int s = input % radix;
		input = (input - s)/radix;

		swap(rank[counter], rank[counter + s]);
	}
}

void mw_kemeny2_34e::print_rank(const std::vector<int> & rank) const {

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
pair<int, int> mw_kemeny2_34e::distance_to_closest(const std::vector<int> &
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

int mw_kemeny2_34e::get_score(const std::vector<q_ballot> & ballots,
	const std::vector<std::vector<int> > & centroids,
	std::vector<int> & support, int numcands) const {

	int sum = 0;

	for (size_t counter = 0; counter < ballots.size(); ++counter) {
		pair<int, int> component = distance_to_closest(ballots[counter].
				rank, centroids, numcands);

		sum += ballots[counter].strength * component.first;
		support[component.second]++;

		//	cout << counter << "("; print(ballots[counter].rank); cout << ") is closest to " << component.second << " with " << ballots[counter].strength << " * " << component.first << " = " << ballots[counter].strength * component.first << endl;
	}
	//cout << endl;

	return (sum);
}

pair<bool, std::list<size_t> > mw_kemeny2_34e::verify(
	const std::vector<std::vector<int> > & centroids,
	const std::vector<int> & support, int numcands,
	bool fill_list) const {
	// Proportionality hack. This is more like divisor proportionality
	// than Droop proportionality: we take the fractions closest to each
	// ordering and adjust the divisor so the sum is equal to the number
	// of seats. Then we take the appropriate number from each ranking and
	// see if any single member appears twice. If so, verification fails,
	// otherwise succeeds.
	// SLOW, but PoC.

	// No. We can just use Sainte-Laguë.
	std::vector<int> iters(centroids.size(), 0);
	std::vector<bool> seen(numcands, false);
	int total_iter;

	int numseats = centroids.size();
	std::list<size_t> positions;

	//cout << "CALL!" << endl;

	for (total_iter = 0; total_iter < numseats; ++total_iter) {
		//if (fill_list)
		//	cout << "Support[" << total_iter << "] = " << support[total_iter] << endl;
		int recordholder = 0;
		for (size_t counter = 1; counter < centroids.size(); ++counter) {
			if (support[counter]/(2.0*iters[counter]+1) >
				support[recordholder]/
				(2.0*iters[recordholder]+1)) {
				recordholder = counter;
			}
		}

		/*if (fill_list)
		cout << "Picked " << iters[recordholder] << "th from " <<
			recordholder << " with adj. support " <<
			support[recordholder] / (2.0 * iters[recordholder]+1)
			<< endl;*/


		int cand = centroids[recordholder][iters[recordholder]++];

		if (fill_list) {
			positions.push_back(cand);
		}
		if (seen[cand]) {
			return (pair<bool, std::list<size_t> >(false, positions));
		} else	{
			seen[cand] = true;
		}
	}

	return (pair<bool, std::list<size_t> >(true, positions)); // for now
}

q_ballot mw_kemeny2_34e::build_ballot(int strength, string order) const {

	q_ballot toret;
	toret.strength = strength;

	for (size_t counter = 0; counter < order.size(); ++counter) {
		toret.rank.push_back(order[counter] - 'A');
	}

	return (toret);
}

int mw_kemeny2_34e::factorial(int n) const {
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
void mw_kemeny2_34e::recurse_ranking(const std::vector<q_ballot> & ballots,
	const std::vector<int> & arch,
	std::vector<std::vector<int> > & recordholder, int & record,
	std::vector<std::vector<int> > & centroids, int pos,
	int lastnum, int numcands, int fact) const {

	int counter;
	int maximum = centroids.size();

	/*if (pos != maximum) {
		std::cout << "MW-Kemeny: Recursing on " << pos << " of " << maximum << std::endl;
	}*/

	if (pos == maximum) {
		// REQUIRED for number ten!
		// Why? Find out.
		std::vector<int> support(centroids.size(), 0);
		int cand = get_score(ballots, centroids, support, numcands);

		if (cand > record  && record != -1) {
			return;    // no point verifying
		}

		if (!verify(centroids, support,  numcands, false).first) {
			return;
		}

		//cout << "Passed and stuff" << endl;

		bool replace_with_new = cand < record || record == -1;
		// Run improvised tiebreak. Whoever has the least distance
		// between centroids wins (or should this be greatest?)
		// (Must be > to pass the DPC?)
		if (!replace_with_new && cand == record)
			replace_with_new = (tiebreak(centroids, numcands) >
					tiebreak(recordholder, numcands));

		// Beware ties
		if (replace_with_new) {
			//		cout << "Replace" << endl;
			record = cand;
			recordholder = centroids;
		}

		return;
	}

	//if (offset == numcands) return;
	//if (numcands - offset < maximum - pos) return; // not enough space

	// Recurse!
	// See other example for why we do it like this
	//recurse_ranking(ballots, arch, recordholder, record, centroids, pos,
	//		offset+1, numcands, fact);

	for (counter = lastnum; counter < fact; ++counter) {
		//if (pos == 0) cout << counter << " of " << fact << endl;
		centroids[pos] = arch;
		permutation(counter, centroids[pos]);

		recurse_ranking(ballots, arch, recordholder, record,
			centroids, pos+1, lastnum+1, numcands, fact);
	}
}


// Make properly recursive later

std::list<size_t> mw_kemeny2_34e::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & vballots) const {

	election_t compressed_ballots = btools.compress(vballots);

	std::vector<int> rank(num_candidates, 0);
	iota(rank.begin(), rank.end(), 0);

	assert(num_candidates < 12);

	std::vector<q_ballot> xlat_ballots;
	// translate here
	// Doesn't handle equal rank, etc.
	for (election_t::const_iterator pos = compressed_ballots.begin();
		pos != compressed_ballots.end(); ++pos) {
		q_ballot next_one;
		next_one.strength = pos->get_weight();
		for (ordering::const_iterator spos = pos->contents.begin();
			spos != pos->contents.end(); ++spos) {
			next_one.rank.push_back(spos->get_candidate_num());
		}
		xlat_ballots.push_back(next_one);
	}

	int record = -1;
	std::vector<std::vector<int> > recordholder(council_size,
		std::vector<int>(num_candidates, -1));
	std::vector<std::vector<int> > centroids(council_size);

	recurse_ranking(xlat_ballots, rank, recordholder, record, centroids, 0,
		0, num_candidates, factorial(num_candidates));

	std::vector<int> support(centroids.size(), 0);
	/*int cand = get_score(xlat_ballots, recordholder, support,
			num_candidates);*/
	return (verify(recordholder, support, num_candidates, true).second);

	std::list<size_t> toRet;

	std::vector<int> already(num_candidates, false);

	for (size_t counter = 0; counter < recordholder.size(); ++counter) {
		toRet.push_back(recordholder[counter][0]);
		assert(!already[recordholder[counter][0]]);
		already[recordholder[counter][0]] = true;
	}

	return (toRet);
}