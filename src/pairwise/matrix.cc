#include "../ballots.h"

#include "types.h"

#include "abstract_matrix.h"
#include "matrix.h"

#include <iterator>
#include <iostream>

using namespace std;

// Condorcet matrix, the Implementation.

double condmat::get_internal(int candidate, int against, bool raw) const {
	// If outside of bounds, kick and shout.
	// Too slow, enable it if you wish.
	/*
	if (candidate < against)
		assert (candidate >= 0 && against < contents.size());
	else	assert (against >= 0 && candidate < contents.size());*/

	if (raw)
		return(contents[candidate][against]);
	else	return(type.transform(contents[candidate][against],
				contents[against][candidate], num_voters));
}

bool condmat::set_internal(int candidate, int against, double value) {
	// Again, if outside of bounds, yell very loudly.
	if (candidate < against)
		assert (candidate >= 0 && against < (int)contents.size());
	else	assert (against >= 0 && candidate < (int)contents.size());

	contents[candidate][against] = value;

	return(true);
}

condmat::condmat(pairwise_type type_in) : abstract_condmat(type_in) {}

condmat::condmat(const list<ballot_group> & scores, int num_candidates,
		pairwise_type kind) : abstract_condmat(kind) {

	assert (num_candidates > 1);

	contents = vector<vector<double> >(num_candidates, 
			vector<double>(num_candidates, 0));

	count_ballots(scores, num_candidates);
}

condmat::condmat(int num_candidates_in, double num_voters_in,
		pairwise_type type_in) : abstract_condmat(type_in) {

	assert(num_candidates_in > 1);

	contents = vector<vector<double> > (num_candidates_in,
			vector<double>(num_candidates_in, 0));

	num_voters = num_voters_in;
}

condmat::condmat(const condmat & in, pairwise_type type_in) : abstract_condmat(
		type_in) {
	contents = in.contents;
	num_voters = in.num_voters;
}

// count ballots go here. Then test.

// Perhaps bool clear.
void condmat::count_ballots(const list<ballot_group> & scores,
		int num_candidates) {

	// For each ballot
	//	 For each pair of candidates
	//		If the candidates have different score,
	//			add the appropriate weight to the matrix at
	//			the appropriate position.

	// We handle incomplete ballots by storing a v<bool> of num_candidates.
	// Every candidate met gets marked as true in it, and then we simply
	// add weight points to all trues above all falses afterwards. We do
	// nothing if we've seen all the candidates (i.e. ballot is complete).
	
	if ((int)contents.size() != num_candidates)
		contents = vector<vector<double> > (num_candidates,
				vector<double> (num_candidates, 0));

	vector<bool> seen(num_candidates);

	bool debug = false;
	num_voters = 0;

	for (list<ballot_group>::const_iterator ballot = scores.begin();
			ballot != scores.end(); ++ballot) {

		fill(seen.begin(), seen.end(), false);
		int seen_candidates = 0;

		ordering::const_iterator cand, against;

		num_voters += ballot->weight;

		for (ordering::const_iterator cand = ballot->contents.begin();
				cand != ballot->contents.end(); ++cand) {

			// Check that the data is valid.
			assert(cand->get_candidate_num() >= 0 &&
					cand->get_candidate_num() < 
					num_candidates);

			// Okay, it's valid, so mark it off...
			if (!seen[cand->get_candidate_num()]) {
				seen[cand->get_candidate_num()] = true;
				++seen_candidates;
			}

			// ... and count all lower candidates against it.
			against = cand;

			while (++against != ballot->contents.end()) {
				// If it has the same score as the other
				// candidate, there's no need to deal with it,
				// so skip.
				if (against->get_score() == cand->get_score())
					continue;

				assert(against->get_candidate_num() >= 0 &&
						against->get_candidate_num() <
						num_candidates);

				// Okay, it's legitimate and there's a 
				// strict preference. Add it.
				double last = get_internal(cand->
						get_candidate_num(),
						against->get_candidate_num(),
						true);

				if (debug)
					cout << "Adding "<< ballot->weight <<
						" to " 	<< (char)('A' + cand->
							get_candidate_num()) 
						<< " vs " << (char)('A' + 
							against->
							get_candidate_num()) 
						<< " -- was " << last 
						<< " is now ";

				set_internal(cand->get_candidate_num(),
						against->get_candidate_num(),
						last + ballot->weight);

				if (debug) {
					cout << get_internal(cand->
							get_candidate_num(),
							against->
							get_candidate_num(),
							true);
					cout << endl;
				}
			}
		}

		// If this was an incomplete ballot, complete it by ranking
		// all those listed above all those not.
		if (seen_candidates != num_candidates) {

			for (size_t counter = 0; counter < seen.size(); 
				++counter){
				// If it's not a ranked candidate, skip
				if (!seen[counter])
					continue;

				for (size_t sec = 0; sec < seen.size(); ++sec) {
					// If it's a ranked candidate, skip
					if (seen[sec])
						continue;

					if (debug)
						cout << "Unranked vs ranked: "<<
							"adding " << ballot->
							weight 	<< " to " <<
							(char)('A'+ counter) <<
							" vs " << 
							(char)('A' + sec) 
							<< endl;

					// The ranked candidate beats the
					// unranked one - update scores.

					set_internal(counter, sec,
							get_internal(counter,
								sec, true) +
							ballot->weight);
				}
			}
		}

		if (debug)
			cout << endl;
	}

	if (debug) {
		cout << "After count-ballots:" << endl;
		for (int y = 0; y < num_candidates; ++y) {
			for (int x = 0; x < num_candidates; ++x)
				cout << get_internal(y, x, true) << "    ";
			cout << endl;
		}
	}
}

void condmat::zeroize() {
	
	for (vector<vector<double> > ::iterator outer = contents.begin();
			outer != contents.end(); ++outer)
		fill(outer->begin(), outer->end(), 0);

}
