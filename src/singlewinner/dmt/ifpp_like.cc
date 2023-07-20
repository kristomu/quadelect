#include "ifpp_like.h"

#include "../../pairwise/matrix.h"
#include "../positional/simple_methods.h"

std::pair<ordering, bool> ifpp_like_fpa_fpc::elect_inner(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	plurality plurality_eval(PT_FRACTIONAL);

	double numvoters = 0;
	std::list<ballot_group>::const_iterator pos;
	for (pos = papers.begin(); pos != papers.end(); ++pos) {
		numvoters += pos->get_weight();
	}

	ordering mod_plurality_outcome = plurality_eval.elect(papers, hopefuls,
			num_candidates, cache, winner_only);

	// Check if the two highest ranked candidates have more than a third
	// of the first preferences each. Since at most two voters can be in
	// this situation, ties naturally sort themselves out.

	ordering::const_iterator outcome_pos = mod_plurality_outcome.begin();
	candscore first = *outcome_pos++;
	candscore second = *outcome_pos;

	// With more than three candidates, this method sucks because nobody's
	// going to be a DMT candidate. I have to find a more general criterion
	// that gives burial resistance. Perhaps this plus some reasonable sort
	// of clone dependence?

	// TODO: Generalize this method so we get contingent vote for free.

	if (3 * first.get_score() > numvoters &&
		3 * second.get_score() > numvoters) {

		condmat pairwise_matrix(papers, num_candidates, CM_PAIRWISE_OPP);

		double first_over_sec = pairwise_matrix.get_magnitude(
				first.get_candidate_num(), second.get_candidate_num(),
				hopefuls);

		double sec_over_first = pairwise_matrix.get_magnitude(
				second.get_candidate_num(), first.get_candidate_num(),
				hopefuls);

		// Delete the top two elements from the modified outcome;
		// we'll reinsert them after this.
		mod_plurality_outcome.erase(mod_plurality_outcome.begin());
		mod_plurality_outcome.erase(mod_plurality_outcome.begin());

		// If there's a pairwise tie between A and B, do nothing;
		// in that case the Plurality winner wins.

		if (first_over_sec > sec_over_first) {
			first.set_score(first.get_score() + numvoters);
		} else if (sec_over_first > first_over_sec) {
			second.set_score(second.get_score() + numvoters);
		}

		mod_plurality_outcome.insert(second);
		mod_plurality_outcome.insert(first);
	}

	return std::pair<ordering, bool>(mod_plurality_outcome, false);
}