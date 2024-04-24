
#include "condorcet.h"

int condorcet_or_loser::get_extreme_candidate(
	const abstract_condmat & input,
	bool get_winner, const std::vector<bool> & hopefuls) const {

	// The algorithm description below is written as if we're finding the
	// Condorcet winner. If we want the loser, we just flip the sign on every
	// comparison.

	// First, we go through every candidate, where the winner of a one-on
	// one between the first and second in line wins. If there's a CW, we'll
	// end up with the CW.

	int incumbent = 0, challenger;

	// Find the first hopeful candidate
	for (incumbent = 0; incumbent < input.get_num_candidates() &&
		!hopefuls[incumbent]; ++incumbent) {
	}

	// If none, get outta here.
	if (incumbent == input.get_num_candidates()) {
		return -1;
	}

	for (challenger = incumbent+1; challenger < input.get_num_candidates();
		++challenger) {

		if (!hopefuls[challenger]) {
			continue;
		}

		bool switch_to_new = false;

		if (get_winner) {
			switch_to_new = input.beats(challenger, incumbent);
		} else {
			switch_to_new = input.beats(incumbent, challenger);
		}

		if (switch_to_new) {
			incumbent = challenger;
		}
	}

	// Then, we check that candidate's results against everybody else.
	// If he wins against all of them, he's the CW, otherwise there's a
	// cycle and we should return -1.

	for (challenger = 0; challenger < input.get_num_candidates();
		++challenger) {
		if (!hopefuls[challenger] || challenger == incumbent) {
			continue;
		}

		if (get_winner) {
			if (input.beats_or_ties(challenger, incumbent)) {
				// Not the CW after all.
				return -1;
			}
		} else {
			if (input.beats_or_ties(incumbent, challenger)) {
				// Not the loser after all.
				return -1;
			}
		}
	}

	return incumbent;
}

std::pair<ordering, bool> condorcet_or_loser::pair_elect(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	// As above, the algorithm below is described as if we're looking
	// for the Condorcet winner. If we're looking for the loser, then this
	// algorithm increases the current rank as it goes instead of decreasing
	// it. This then inserts the losers from the bottom up instead of
	// the top down.

	// We start with the hopefuls we have. Find the CW, insert into the
	// output ordering below what's already there, mark as no longer
	// hopeful, and repeat. Stop when we get a -1 or all candidates have
	// been added.

	std::vector<bool> iter_hopefuls = hopefuls;
	std::vector<bool> extremes(input.get_num_candidates(), false);
	ordering to_ret;
	int rank = 0, extreme;
	size_t counter = 0;

	do {
		extreme = get_extreme_candidate(input, winner,
				iter_hopefuls);

		if (extreme != -1) {
			if (winner) {
				to_ret.insert(candscore(extreme, rank--));
			} else {
				to_ret.insert(candscore(extreme, rank++));
			}
			iter_hopefuls[extreme] = false;
			extremes[extreme] = true;
			++counter;
		}

	} while (extreme != -1 && !winner_only &&
		counter < input.get_num_candidates());

	// Add all the other candidates below the iterated CWs.

	for (counter = 0; counter < extremes.size(); ++counter) {
		if (iter_hopefuls[counter] && !extremes[counter]) {
			to_ret.insert(candscore(counter, rank));
		}
	}

	return (std::pair<ordering, bool>(to_ret, winner_only));
}

// Condorcet non-loser. Everything is analogous, really.