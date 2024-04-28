#include "no_elimination_irv.h"

std::vector<double> no_elimination_irv::get_scores(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates) const {

	std::vector<bool> pseudo_eliminated(num_candidates, false);
	int num_remaining_candidates = 0;

	for (bool hopeful: hopefuls) {
		if (hopeful) {
			++num_remaining_candidates;
		}
	}

	ordering::const_iterator pos;

	std::vector<double> scores(num_candidates, -1);

	bool is_type_one = true; // <-- HACK
	int cand;

	for (;;) {

		std::vector<double> approvals(num_candidates, 0);
		double numvoters = 0;

		for (const ballot_group & ballot: papers) {
			// Descend down the ballot's preference list to find the
			// score of the first candidate that hasn't been pseudo-
			// eliminated.

			bool non_eliminated_found = false;
			double threshold_score = 0;

			numvoters += ballot.get_weight();

			for (pos = ballot.contents.begin(); pos != ballot.contents.end()
				&& !non_eliminated_found; ++pos) {

				if (!hopefuls[pos->get_candidate_num()]) {
					continue;
				}

				// Update the score so that when we abort, it will be
				// the score of the first candidate who wasn't eliminated,
				// or the lowest score if everybody is.
				threshold_score = pos->get_score();

				if (!pseudo_eliminated[pos->get_candidate_num()]) {
					non_eliminated_found = true;
				}
			}

			// Now update the approval counts. We do this in two steps
			// to properly handle equal-rank.
			for (pos = ballot.contents.begin(); pos != ballot.contents.end() &&
				pos->get_score() >= threshold_score; ++pos) {

				if (!hopefuls[pos->get_candidate_num()]) {
					continue;
				}

				approvals[pos->get_candidate_num()] += ballot.get_weight();
			}
		}

		// If only a single candidate is uneliminated and this is type 1,
		// return that candidate as the winner (approval = numcands): order
		// all the other candidates by approval count.

		// This might not strictly be the correct thing to do, check later
		// with Kevin's definition.
		// TODO. (It isn't and it's probably what leads to such awful Benham-Meta performance. We should return in order of pseudo-elimination instead.)

		if (num_remaining_candidates == 1 && is_type_one) {
			for (cand = 0; cand < num_candidates; ++cand) {
				if (!hopefuls[cand]) {
					continue;
				}
				if (pseudo_eliminated[cand]) {
					scores[cand] = approvals[cand];
				} else {
					scores[cand] = numvoters;
				}
			}
			return scores;
		}

		// If at least one candidate has support > 50%, return them all as
		// winners.
		bool any_majorities = false;
		for (cand = 0; cand < num_candidates && !any_majorities; ++cand) {
			if (!hopefuls[cand]) {
				continue;
			}
			any_majorities &= approvals[cand] <= numvoters/2.0;
		}

		if (any_majorities) {
			for (cand = 0; cand < num_candidates; ++cand) {
				if (!hopefuls[cand]) {
					continue;
				}
				if (approvals[cand] <= numvoters/2.0) {
					scores[cand] = approvals[cand];
				} else {
					scores[cand] = numvoters;
				}
			}
			return scores;
		}

		// If no candidate is uneliminated, return the outcome by approval count.
		if (num_remaining_candidates == 0) {
			return approvals;
		}

		// Otherwise, eliminate the loser and loop.
		int loser_idx = -1;
		double loser_score = 0;

		for (cand = 0; cand < num_candidates; ++cand) {
			if (!hopefuls[cand] || pseudo_eliminated[cand]) {
				continue;
			}

			// WARNING: breaks neutrality. fix later.
			if (loser_idx == -1 || loser_score >= approvals[cand]) {
				loser_score = approvals[cand];
				loser_idx = cand;
			}
		}

		pseudo_eliminated[loser_idx] = true;
		--num_remaining_candidates;
	}
}

std::pair<ordering, bool> no_elimination_irv::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	std::vector<double> scores = get_scores(papers, hopefuls,
			num_candidates);

	ordering outcome;

	for (int i = 0; i < num_candidates; ++i) {
		if (hopefuls[i]) {
			outcome.insert(candscore(i, scores[i]));
		}
	}

	return (std::pair<ordering, bool>(outcome, false));
}
