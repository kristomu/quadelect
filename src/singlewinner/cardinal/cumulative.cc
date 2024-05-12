#include "cumulative.h"

#include <iostream>

// Scales all the scores to [0..1] before the p-norm normalization so
// that we can pass scale invariance. See the header for more info.

std::pair<ordering, bool> cumulative_voting::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	std::vector<double> scores(num_candidates, 0);

	for (const ballot_group & b: papers) {

		ordering ratings = b.contents;

		// Determine the highest and lowest ratings provided.
		bool found_candidate = false;
		double bottom_score = 0, top_score = 0;

		for (const candscore & cs: ratings) {
			if (!hopefuls[cs.get_candidate_num()]) {
				continue;
			}

			if (!found_candidate) {
				found_candidate = true;
				bottom_score = cs.get_score();
				top_score = cs.get_score();
			}

			bottom_score = std::min(bottom_score,
					cs.get_score());
			top_score = std::max(top_score, cs.get_score());
		}

		// Exhausted ballot - skip.
		if (!found_candidate) {
			continue;
		}

		// Calculate the Lp normalization value.

		double p_val = 0;

		for (const candscore & cs: ratings) {
			if (!hopefuls[cs.get_candidate_num()]) {
				continue;
			}

			double norm_score = norm(bottom_score,
					cs.get_score(), top_score);

			p_val += pow(norm_score, p);
			assert(finite(p_val));
		}

		p_val = pow(p_val, 1.0/p);

		if (p_val == 0) {
			continue;
		}

		// Add the ballot ratings to the scores tally.

		for (const candscore & cs: ratings) {
			if (!hopefuls[cs.get_candidate_num()]) {
				continue;
			}

			double norm_score = norm(bottom_score,
					cs.get_score(), top_score);

			scores[cs.get_candidate_num()] += b.get_weight() *
				round(10 * norm_score / p_val);

			assert(finite(scores[cs.get_candidate_num()]));
		}
	}

	// Now just dump by score.

	ordering out;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		out.insert(candscore(cand, scores[cand]));
	}

	return std::pair<ordering, bool>(out, false);
}
