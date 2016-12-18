// "Dirichlet model": Ranks according to scores v_a + v_b + ... = 1 with all
// v scores >= 0.
// Again, unsure if this is random for truncated ballots.

#include "dirichlet.h"
#include <ext/numeric>

using namespace std;

ordering dirichlet::generate_ordering(int numcands, bool do_truncate,
		rng & random_source) const {

	// First generate a random order, as with impartial. Cut them off if
	// truncate is specified, then generate scores which we normalize to
	// 1 (not strictly needed unless you're tinkering with ratings, but
	// we want to be according to spec).

	// Shouldn't shuffle, I think. TODO, fix this entire thing. I think
	// Dirichlet is just: every candidate gets a random score.

	size_t how_far;
	if (truncate && numcands > 1)
		how_far = random_source.irand(1, numcands+1);
	else	how_far = 1;

	vector<double> scores(how_far, 0);
	size_t counter;
	double sum = 0;
	for (counter = 0; counter < how_far; ++counter) {
		scores[counter] = random_source.drand();
		sum += scores[counter];
	}

	ordering toRet;

	// If we aren't going to truncate, then there's no need to shuffle as
	// the drand does that automatically.

	if ((int)how_far != numcands) {
		vector<int> candidates(numcands, 0);
		iota(candidates.begin(), candidates.end(), 0);
		random_shuffle(candidates.begin(), candidates.end(), 
				random_source);

		for (counter = 0; counter < how_far; ++counter)
			toRet.insert(candscore(candidates[counter],
						scores[counter]/sum));
	} else {
		for (counter = 0; counter < how_far; ++counter)
			toRet.insert(candscore(counter, 
						scores[counter]/sum));
	}

	return(toRet);
}
