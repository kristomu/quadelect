#include "dhwl.h"

std::list<size_t> reweighted_condorcet::get_council(size_t council_size,
	size_t num_candidates, const election_t & ballots) const {

	if (council_size > num_candidates) {
		throw std::invalid_argument("multiwinner: Too few candidates for the council");
	}

	// Until the council has been filled: Run a DHwL_matrix election
	// with the given Condorcet method. Pick the first non-elected candidate
	// in the social order, add him to the council, and set that one as
	// elected.

	std::list<size_t> council;
	size_t council_count = 0;

	std::vector<bool> elected(num_candidates, false);

	while (council_count < council_size) {

		// TODO: Fix later. DHwL no longer implements it.
		bool tie_at_top = false;

		ordering social_order = base->pair_elect(DHwLmatrix(ballots,
					elected, num_candidates,
					base->get_type(), tie_at_top, 0.5), false).first;

		int next_candidate = -1;

		for (ordering::const_iterator pos = social_order.begin(); pos !=
			social_order.end() && next_candidate == -1; ++pos) {

			if (!elected[pos->get_candidate_num()]) {
				next_candidate = pos->get_candidate_num();
			}
		}

		assert(next_candidate != -1);

		council.push_back(next_candidate);
		elected[next_candidate] = true;
		++council_count;
	}

	return (council);
}
