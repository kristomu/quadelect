#include "ballots_by_support.h"
#include <limits>
#include <numeric>

void ballots_by_support::group_by_support(
	const election_t & ballots,
	size_t winner_in, size_t challenger_in) {

	reset(winner_in, challenger_in);

	for (const ballot_group & b_group : ballots) {
		bool seen_winner = false, seen_challenger = false;

		// Scores are initialized to -infinity so that ranked
		// candidates always beat unranked ones.
		double winner_score = -std::numeric_limits<double>::infinity(),
			   challenger_score = -std::numeric_limits<double>::infinity();

		// Find the rank for the challenger and the winner.
		// Note that these count from the top, so challenger < winner
		// means that the challenger is ranked higher than the
		// winner.

		for (auto pos = b_group.contents.begin(); pos !=
			b_group.contents.end() &&
			(!seen_winner || !seen_challenger); ++pos) {

			if (pos->get_candidate_num() == winner) {
				winner_score = pos->get_score();
				seen_winner = true;
			}
			if (pos->get_candidate_num() == challenger) {
				challenger_score = pos->get_score();
				seen_challenger = true;
			}
		}

		// We consider equal-ranks to be in favor of the winner,
		// thus we only explicitly check if the challenger was
		// rated above the winner.
		if (challenger_score > winner_score) {
			challenger_support += b_group.get_weight();
			supporting_challenger.push_back(b_group);
		} else {
			other_support += b_group.get_weight();
			others.push_back(b_group);
		}
	}
}
