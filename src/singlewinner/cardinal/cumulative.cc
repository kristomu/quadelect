#include "cumulative.h"

#include <iostream>

// TODO: Somehow retain scale invariance, because otherwise a ballot
// of the type A: 100, B: 50: C: 25 would be treated differently from
// A: 10, B: 5, C: 2.5, which we don't want.

// Probably just scale all the scores to [0..1] before the p-norm
// normalization. But I have to actually *do* it. :-P

std::pair<ordering, bool> cumulative_voting::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	std::vector<double> scores(num_candidates, 0);

	for (const ballot_group & b: papers) {

		ordering ratings = b.contents;

		// Determine the lowest rated hopeful candidate.
		bool found_candidate = false;
		double bottom_score;

		for (auto rev_pos = ratings.rbegin();
			rev_pos != ratings.rend() && !found_candidate;
			++rev_pos) {

			if (!hopefuls[rev_pos->get_candidate_num()]) {
				continue;
			}

			found_candidate = true;
			bottom_score = rev_pos->get_score();
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

			p_val += pow(cs.get_score() - bottom_score, p);
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

			scores[cs.get_candidate_num()] += b.get_weight() *
				(cs.get_score() - bottom_score) / p_val;

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