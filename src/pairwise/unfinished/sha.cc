// This returns a very hard to manipulate matrix that is still neutral with 
// respect to candidates. Methods using it will fail just about every criterion,
// however.

// The ordering is hard to manipulate in the sense that it's computationally
// challenging to find a strategy that puts the desired candidate at the top,
// in general.
// However, it is not challenging for a small number of candidates, as it can
// simply be bruteforced.

// The matrix and orderings produced by running it through good methods are not
// designed to be used alone, but rather for tournament seedings and prerounds,
// and for miscellaneous other tiebreakers to see which criteria hold for the 
// base method no matter the seeding/etc. It's randomness without randomness,
// basically.

// The key concept is to order pairwise rankings by magnitude (thus ensuring
// neutrality), and then to run the magnitudes, in sorted order, along with a 
// number 1 ..n, through SHA-256. The output bit sequences define whether the 
// winner ranks above the loser.

// BLUESKY: Make this into random_condmat which can either be truly random or
// derived from the votes, and then have a secure PRNG class provide the SHA 
// part.

#ifndef _VOTE_C_SHASTRAT
#define _VOTE_C_SHASTRAT

#include "../../ballots.h"
#include "../../tools/tools.h"
#include "../matrix.h"

#include <vector>
#include <list>

#include <openssl/sha.h>

using namespace std;

class sha_condmat : public abstract_condmat {
	private:
		vector<vector<double> > contents;

		double get_hash_value(string instr, int count) const;

		void update_contents(const abstract_condmat & in,
				const vector<bool> & hopefuls);

	protected:
		double get_internal(int candidate, int against, bool raw) const;
		bool set_internal(int candidate, int against, double value);

		// Public to come.

};

// Given an input string (a "seed"), and a count (number of candidate pair in
// the sorted list), return a double-precision (52 bit) hash value derived from
// the SHA-256 output of the string and count. Returns INFINITY if something's
// wrong.
// This would be even more secure if we used a block cipher in counter mode, as
// SHAs "x"+1, "x"+2 etc may be related in some way whereas block cipher results
// wouldn't be. Oh well, fix later?
double sha_condmat::get_hash_value(string instr, int count) const {

	instr += "#" + itos(count);

	unsigned char * output = (unsigned char *)calloc(256/8, sizeof(char));

	if (output == NULL) 
		return(INFINITY);

	unsigned char * input = (unsigned char *)calloc(instr.size()+1, 
			sizeof(char));

	if (input == NULL) {
		free(output);
		return(INFINITY);
	}

	copy(instr.begin(), instr.end(), input);

	if (SHA256(input, instr.size(), output) == NULL) {
		free(output);
		return(INFINITY);
	}

	uint64_t value_integer = 1;
	
	for (int counter = 0; counter < 8; ++counter) {
		value_integer <<= 8;
		value_integer += output[counter];
	}

	uint64_t maxVal = ~((uint64_t)0); // Maximum value we can get.

	long double RS_SCALE = (1.0 / (1.0 + maxVal));

	return(value_integer * RS_SCALE);
}

void sha_condmat::update_contents(const abstract_condmat & in,
		const vector<bool> & hopefuls) {

	list<pair<double, pair<int, int> > > scores;

	// Get the pairwise scores.
	for (int counter = 0; counter < in.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) continue;

		for (int sec = 0; sec < in.get_num_candidates(); ++sec) {
			if (!hopefuls[sec] || counter == sec) continue;

			pair<double, pair<int, int> > result;
			result.second = pair<int,int>(counter, sec);
			result.first = in.get_magnitude(counter, sec,
					hopefuls);
		}
	}

	// Sort them. Note that this isn't neutral if there are ties. Perhaps
	// fix that by including every permutation of the ties, adding up 
	// magnitudes, or somesuch.

	scores.sort(greater<pair<double, pair<int, int> > >());

	// To strive towards being implementation-independent, use dtos to make
	// a string of the magnitude values. It will still be implementation-
	// dependent, though.

	list<pair<double, pair<int, int> > >::const_iterator pos;
	
	string hash_source;

	for (pos = scores.begin(); pos != scores.end(); ++pos)
		hash_source += "|" + dtos(pos->first);
}
#endif

