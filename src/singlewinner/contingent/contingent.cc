#include "contingent.h"

#include "../../pairwise/matrix.h"
#include "../positional/simple_methods.h"

double contingent_vote::get_possible_finalist_score(
	size_t finalist, size_t challenger,
	const condmat & pairwise_matrix,
	const std::vector<bool> & hopefuls,
	const std::vector<double> & plur_scores) const {

	return pairwise_matrix.get_magnitude(finalist,
			challenger);
}

std::pair<ordering, bool> contingent_vote::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// TODO? Expose the fractional/whole parameter to the outside.

	plurality plurality_eval(PT_FRACTIONAL);
	condmat pairwise_matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	double numvoters = 0;
	election_t::const_iterator ballot_pos;

	for (ballot_pos = papers.begin(); ballot_pos != papers.end();
		++ballot_pos) {
		numvoters += ballot_pos->get_weight();
	}

	ordering plurality_outcome = plurality_eval.elect(papers, hopefuls,
			num_candidates, cache, false);

	std::vector<double> scores(num_candidates, 0),
		plur_scores(num_candidates, 0);

	// First transform the Plurality outcome into a form that's easier to
	// access. (I've used this lots of times now, should I extract?)

	ordering::const_iterator pos;
	for (pos = plurality_outcome.begin(); pos !=
		plurality_outcome.end(); ++pos) {
		plur_scores[pos->get_candidate_num()] = pos->get_score();
	}

	// Determine the finalist set.

	std::vector<size_t> finalists;

	double plurality_winner_score, plurality_second_place_score;
	plurality_winner_score = plurality_outcome.begin()->get_score();

	// Get the winners.
	for (pos = plurality_outcome.begin(); pos != plurality_outcome.end()
		&& pos->get_score() == plurality_winner_score; ++pos) {
		if (hopefuls[pos->get_candidate_num()]) {
			finalists.push_back(pos->get_candidate_num());
		}
	}

	// Now pos is at the first second-place finisher, or at the end if
	// it's a full tie.

	// If we have multiple Plurality winners, then the second-placers
	// can't possibly win. Thus we shouldn't add them to the finalists
	// set.
	if (pos != plurality_outcome.end() && finalists.size() < 2) {
		plurality_second_place_score = pos->get_score();

		for (; pos != plurality_outcome.end() && pos->get_score() ==
			plurality_second_place_score; ++pos) {
			if (hopefuls[pos->get_candidate_num()]) {
				finalists.push_back(pos->get_candidate_num());
			}
		}
	}

	// Calculate the finalists' scores.

	ordering outcome;

	for (const size_t & finalist: finalists) {
		if (!hopefuls[finalist]) {
			continue;
		}
		for (const size_t & challenger: finalists) {
			if (finalist == challenger ||
				!hopefuls[challenger]) {
				continue;
			}

			// If neither of the two candidates provided has the
			// maximum Plurality score, then this pair can't be an
			// actual finalist pair because at least one of the
			// candidates must've been eliminated; so skip it.

			if (plur_scores[finalist] < plurality_winner_score &&
				plur_scores[challenger] < plurality_winner_score) {
				continue;
			}

			scores[finalist] = std::max(scores[finalist],
					get_possible_finalist_score(finalist,
						challenger, pairwise_matrix,
						hopefuls, plur_scores));
		}
	}

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (hopefuls[cand]) {
			outcome.insert(candscore(cand, scores[cand]));
		}
	}

	return std::pair<ordering, bool>(outcome, false);
}