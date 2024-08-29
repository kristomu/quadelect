#include "scored_method.h"

#include <vector>

void scored_method::process_ballots(const
	election_t & ballots, size_t num_candidates) {

	// This is used to calculate birational and LPV results quickly, as
	// those methods have terms like "voter X's rating of candidate Y".
	// With the usual ballot format, that would take at least linear
	// (perhaps quadratic) time, whereas this takes constant.

	// We use NAN for "no opinion" Range-style values.

	scored_ballots = std::vector<scored_ballot>(ballots.size());

	size_t ballot_idx = 0;
	for (election_t::const_iterator pos = ballots.begin();
		pos != ballots.end(); ++pos) {

		scored_ballots[ballot_idx].weight = pos->get_weight();
		scored_ballots[ballot_idx].scores.resize(
			num_candidates, NAN);

		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos) {
			scored_ballots[ballot_idx].scores[opos->get_candidate_num()] =
				opos->get_score();
		}
		++ballot_idx;
	}
}