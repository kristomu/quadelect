#include "sv2.h"
#include "../positional/all.h"
#include "../pairwise/simple_methods.h"

std::pair<ordering, bool> sv_att_second::elect_inner(
	const election_t &
	papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	// Not sure if this is working properly...

	// This is a quick and dirty hack.

	// SV: Let
	//	a = 0.5 - plur(a)/numvoters
	//	b = 0.5 - plur(b)/numvoters
	//	g = 0.5 - plur(c)/numvoters

	// Then if the cycle is ABCA and we're minimizing
	//	A's penalty is b/g 		beaten/third party
	//	B's penalty is g/a
	//	C's penalty is a/b

	// I'll just negate this; can't be bothered.

	assert(num_candidates <= 3);

	// TODO: Use cache.

	condmat condorcet_matrix = condmat(papers, num_candidates,
			CM_PAIRWISE_OPP);

	int num_hopefuls = 0;
	for (bool hopeful : hopefuls) {
		if (hopeful) {
			++num_hopefuls;
		}
	}

	// If two candidates or fewer, just hand off to a method that does a majority vote.
	if (num_hopefuls < 3) {
		return ord_minmax(CM_PAIRWISE_OPP).pair_elect(
				condorcet_matrix, hopefuls, cache, winner_only);
	}

	// Get Plurality scores. Whole or fractional? Should perhaps be set
	// as a parameter...
	// Hopefuls???

	// Why do we have to do it like this???
	election_method * plur = new plurality(PT_WHOLE);
	ordering plur_result = plur->elect(papers, num_candidates, cache,
			false);
	ordering plur_ordering = plur_result;
	delete plur;

	// Do the rest later. Dump plur stuff into a vector, say plurscores,
	// then...

	std::vector<double> plur_scores(num_candidates, 0);
	double numvoters = 0;

	for (ordering::const_iterator pos = plur_ordering.begin(); pos !=
		plur_ordering.end(); ++pos) {
		plur_scores[pos->get_candidate_num()] = pos->get_score();
		numvoters += pos->get_score();
	}

	std::vector<double> plur_factors(num_candidates, 0);
	int counter;
	for (counter = 0; counter < num_candidates; ++counter) {
		plur_factors[counter] = 0.5 - plur_scores[counter]/numvoters;
	}

	/*std::vector<double> scores(num_candidates, 0);

	ordering output;

	for (int i = 0; i < num_candidates; ++i) {
		for (int j = 0; j < num_candidates; ++j)
			if (condorcet_matrix.get_magnitude(i, j) >
					condorcet_matrix.get_magnitude(j, i))
				scores[i] += plur_scores[j];

		output.insert(candscore(i, scores[i]));
	}

	    return(std::pair<ordering, bool>(output, false));*/

	ordering out;

	for (int counter = 0; counter < 3; ++counter) {
		double score = 0;

		// Cycle A>B>C>A
		// then counter = 0 (A)
		// sec = 1 (B),
		// third party = 2 (C)

		// SV would presumably be:
		// (B>C) / (C>A)

		//double majority = 0.5 * input.get_num_voters();

		// EWW!

		int beaten = (counter + 1)%3;
		int third_party = (counter+2)%3;

		// If we beat the third party, then switch the two around.
		if (condorcet_matrix.get_magnitude(counter, third_party) >=
			condorcet_matrix.get_magnitude(third_party, counter)) {

			beaten = (counter+2)%3;
			third_party = (counter+1)%3;
		}

		// If we're a CW, give max score
		if (condorcet_matrix.get_magnitude(counter, third_party) >=
			condorcet_matrix.get_magnitude(third_party, counter)) {
			score = 1000;
		} else {
			score = -(plur_factors[beaten]/plur_factors[third_party]);
		}

		// Set 0/0 to 0
		if (isnan(score)) {
			score = 0;
		}

		out.insert(candscore(counter, score));
	}

	return (std::pair<ordering, bool>(out, false));
}
