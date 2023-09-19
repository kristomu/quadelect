#include "fpa_fpc.h"

#include "../../pairwise/matrix.h"
#include "../positional/simple_methods.h"

std::pair<ordering, bool> fpa_fpc::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	plurality plurality_eval(PT_FRACTIONAL);
	condmat pairwise_matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	double numvoters = 0;
	election_t::const_iterator pos;
	for (pos = papers.begin(); pos != papers.end(); ++pos) {
		numvoters += pos->get_weight();
	}

	ordering plurality_outcome = plurality_eval.elect(papers, hopefuls,
			num_candidates, cache, false);

	std::vector<double> scores(num_candidates, 0),
		plur_scores(num_candidates, 0);

	// First transform the Plurality outcome into a form that's easier to
	// access.

	for (ordering::const_iterator pos = plurality_outcome.begin(); pos !=
		plurality_outcome.end(); ++pos) {
		plur_scores[pos->get_candidate_num()] = pos->get_score();
	}

	// Calculate the score for every candidate.
	int candidate;
	for (candidate = 0; candidate < num_candidates; ++candidate) {
		if (!hopefuls[candidate]) {
			continue;
		}

		double fpA = plur_scores[candidate];
		double combined_fpC = 0;

		for (int challenger = 0; challenger < num_candidates;
			++challenger) {

			if (!hopefuls[challenger] || challenger == candidate) {
				continue;
			}

			if (!pairwise_matrix.beats(challenger, candidate)) {
				continue;
			}

			combined_fpC = aggregate(combined_fpC,
					plur_scores[challenger]);
		}

		scores[candidate] = fpA - combined_fpC;
	}

	ordering outcome;

	for (candidate = 0; candidate < num_candidates; ++candidate) {
		if (hopefuls[candidate]) {
			outcome.insert(candscore(candidate, scores[candidate]));
		}
	}

	return std::pair<ordering, bool>(outcome, false);
}