#include "fpc.h"

#include "../pairwise/matrix.h"

std::pair<ordering, bool> first_pref_copeland::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	condmat pairwise_matrix(papers, num_candidates, CM_PAIRWISE_OPP);
	plurality plurality_eval(PT_FRACTIONAL);

	size_t i, j;
	double numvoters = 0;
	for (election_t::const_iterator pos = papers.begin(); pos !=
		papers.end(); ++pos) {
		numvoters += pos->get_weight();
	}

	ordering plurality_outcome = plurality_eval.elect(papers, hopefuls,
			num_candidates, cache, false);

	std::vector<double> penalties(num_candidates, 0),
		plur_scores(num_candidates, 0);

	// First transform the Plurality outcome into a form that's easier to
	// access.

	for (ordering::const_iterator pos = plurality_outcome.begin(); pos !=
		plurality_outcome.end(); ++pos) {
		plur_scores[pos->get_candidate_num()] = pos->get_score();
	}

	// Then calculate the penalties. We do this in the equivalent of a radix
	// numvoters+1 system; i.e. if A has one first preference vote and beats
	// B, then B's penalty contribution from A is numvoters. If A ties B,
	// then the contribution is 1. This has the effect of incorporating the
	// tiebreaker mentioned in fpc.h without having to deal with vectors,
	// and with the scores (penalties negated) giving some idea as to how
	// "close" a candidate is to another, support wise.

	for (i = 0; i < (size_t)num_candidates; ++i) {
		if (!hopefuls[i]) {
			continue;
		}
		for (j = 0; j < (size_t)num_candidates; ++j) {
			if (!hopefuls[j]) {
				continue;
			}
			if (i == j) {
				continue;
			}

			// If i beats j, add numcands * i's first prefs to j's penalty.
			double IoverJ = pairwise_matrix.get_magnitude(i, j, hopefuls);
			double JoverI = pairwise_matrix.get_magnitude(j, i, hopefuls);

			double potential_penalty = plur_scores[i];

			if (IoverJ > JoverI) {
				penalties[j] += potential_penalty * (numvoters+1);
			}

			if (IoverJ == JoverI) {
				penalties[j] += potential_penalty;
			}
		}
	}

	ordering outcome;

	for (i = 0; i < (size_t)num_candidates; ++i) {
		if (hopefuls[i]) {
			outcome.insert(candscore(i, -penalties[i]));
		}
	}

	return (std::pair<ordering, bool>(outcome, false));
}