#include "donation.h"

double donated_contingent_vote::get_possible_finalist_score(
	size_t finalist, size_t challenger,
	const condmat & pairwise_matrix,
	const std::vector<bool> & hopefuls,
	const std::vector<double> & plur_scores) const {

	double score = pairwise_matrix.get_magnitude(
			finalist, challenger);

	size_t num_candidates = pairwise_matrix.
		get_num_candidates();

	// XXX: It might be a better idea to treat this as a game where the
	// challenger gets a chance to try to boot us out of the top two, and
	// if so, the score is zero. But my preliminary attempts to do so
	// don't seem to produce much of a difference as far as strategic
	// resistance is concerned, so I don't know. Maybe what we have here,
	// just separately "making the case for X", is good enough?


	// Determine the maximum number of voters the finalist can
	// donate. The maximum amount is actually an epsilon less,
	// so we have to be careful with strict inequalities. (I think
	// I got it right.)
	double max_donation = plur_scores[finalist] -
		plur_scores[challenger];

	for (size_t beneficiary = 0; beneficiary < num_candidates;
		++beneficiary) {

		if (beneficiary == finalist || beneficiary == challenger ||
			!hopefuls[beneficiary]) {
			continue;
		}

		// Determine if A can donate to this candidate ("beneficiary").
		// The candidate must initially not be a finalist, but
		// it should be possible to make him one by donating first
		// preferences.
		if (plur_scores[beneficiary] < plur_scores[challenger] &&
			plur_scores[beneficiary] + max_donation >
			plur_scores[challenger]) {

			// OK, we can donate to this candidate, so update our score.
			score = std::max(score,
					pairwise_matrix.get_magnitude(finalist, beneficiary));
		}
	}

	return score;
}