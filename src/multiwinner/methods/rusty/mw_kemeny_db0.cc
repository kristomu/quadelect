#include "mw_kemeny_db0.h"

std::vector<std::vector<bool> > mw_kemeny::tournament_matrix(
	const std::vector<int> & rank,
	size_t maxcand) const {

	std::vector<std::vector<bool> > toRet(maxcand,std::vector<bool>(maxcand,
			false));

	for (size_t counter = 0; counter < rank.size(); ++counter)
		for (size_t sec = counter+1; sec < rank.size(); ++sec) {
			toRet[rank[counter]][rank[sec]] = true;
		}

	return (toRet);
}

int mw_kemeny::p_distance(const std::vector<int> & a,
	const std::vector<int> & b,
	size_t maxcand) const {

	std::vector<std::vector<bool> > cmat_a = tournament_matrix(a, maxcand),
									cmat_b = tournament_matrix(b, maxcand);

	int mismatch_count = 0;

	for (size_t counter = 0; counter < maxcand; ++counter)
		for (size_t sec = counter+1; sec < maxcand; ++sec)
			if (cmat_a[counter][sec] ^ cmat_b[counter][sec]) {
				++mismatch_count;
			}

	return (mismatch_count);
}

// Improvised tiebreak: returns sum distance between centroids (all pairs).
// Is greater better, or less better? We'll try less for now.
int mw_kemeny::tiebreak(const std::vector<std::vector<int> > & a,
	size_t maxcand) const {

	int sum = 0;

	for (size_t counter = 0; counter < a.size(); ++counter)
		for (size_t sec = counter+1; sec < a.size(); ++sec) {
			sum += p_distance(a[counter], a[sec], maxcand);
		}

	return (sum);
}

void mw_kemeny::permutation(int input, std::vector<int> & rank) const {
	for (size_t counter = 0; counter < rank.size(); ++counter) {
		int radix = rank.size() - counter;
		int s = input % radix;
		input = (input - s)/radix;

		std::swap(rank[counter], rank[counter + s]);
	}
}

void mw_kemeny::print_rank(const std::vector<int> & rank) const {

	for (size_t counter = 0; counter < rank.size(); ++counter) {
		if (counter > 0) {
			std::cout << ">";
		}
		std::cout << (char)(rank[counter] + 'A');
	}
	//std::cout << endl;
}

// Returns pair where the first is the record distance and the second is
// the recordholder.
std::pair<int, int> mw_kemeny::distance_to_closest(
	const std::vector<int> & our_rank,
	const std::vector<std::vector<int> > & centroids, int numcands) const {

	std::pair<int, int> record(-1, -1);

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

int mw_kemeny::get_score(const std::vector<q_ballot> & ballots,
	const std::vector<std::vector<int> > & centroids, int numcands) const {

	int sum = 0;

	for (size_t counter = 0; counter < ballots.size(); ++counter) {
		std::pair<int, int> component = distance_to_closest(ballots[counter].
				rank, centroids, numcands);

		sum += ballots[counter].strength * component.first;

		//	cout << counter << "("; print(ballots[counter].rank); cout << ") is closest to " << component.second << " with " << ballots[counter].strength << " * " << component.first << " = " << ballots[counter].strength * component.first << endl;
	}
	//cout << endl;

	return (sum);
}

bool mw_kemeny::verify(const std::vector<std::vector<int> > & centroids,
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

q_ballot mw_kemeny::build_ballot(int strength, std::string order) const {

	q_ballot toret;
	toret.strength = strength;

	for (size_t counter = 0; counter < order.size(); ++counter) {
		toret.rank.push_back(order[counter] - 'A');
	}

	return (toret);
}

int mw_kemeny::factorial(int n) const {
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
void mw_kemeny::recurse_ranking(const std::vector<q_ballot> & ballots,
	const std::vector<int> & arch,
	std::vector<std::vector<int> > & recordholder, int & record,
	std::vector<std::vector<int> > & centroids, int pos,
	int offset, int numcands, int fact) const {

	int maximum = centroids.size();

	if (pos == maximum) {
		// REQUIRED for number ten!
		// Why? Find out.
		if (!verify(centroids, numcands)) {
			return;
		}

		int cand = get_score(ballots, centroids, numcands);

		bool replace_with_new = cand < record || record == -1;
		// Run improvised tiebreak. Whoever has the least distance
		// between centroids wins (or should this be greatest?)
		// (Must be > to pass the DPC?)
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
		offset+1, numcands, fact);

	for (int counter = offset; counter < fact; ++counter) {
		centroids[pos] = arch;
		permutation(counter, centroids[pos]);

		recurse_ranking(ballots, arch, recordholder, record,
			centroids, pos+1, offset+1, numcands, fact);
	}
}


// Make properly recursive later

council_t mw_kemeny::get_council(size_t council_size,
	size_t num_candidates, const election_t & vballots) const {

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

	council_t toRet;

	std::vector<int> already(num_candidates, false);

	for (size_t counter = 0; counter < recordholder.size(); ++counter) {
		toRet.push_back(recordholder[counter][0]);
		assert(!already[recordholder[counter][0]]);
		already[recordholder[counter][0]] = true;
	}

	return (toRet);
}