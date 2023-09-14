// Monotonicity: Implements different types of mono-sub criteria. These are:

// - Mono-sub-plump: Replacing some ballots that do not vote X top with ballots
//                   that vote for X alone shouldn't make X lose.

// - Mono-sub-top:   Replacing some ballots that do not vote X top with ballots
//                   that vote X top (and are otherwise arbitrary) shouldn't
//                   make X lose.

#include "mono_sub.h"

///// Mono-sub-plump
////////////////////

bool mono_sub_plump::alter_ballot(const ordering & input,
	ordering & output,
	size_t numcands, const std::vector<size_t> & data, rng &
	randomizer) const {

	// This one is easy. First check if x is top-ranked. If he isn't,
	// replace by a ballot where he is, otherwise FALSE.

	double highest_score = output.begin()->get_score();
	size_t cand_to_change = data[0];

	ordering::const_iterator pos;
	for (pos = output.begin(); pos != output.end() &&
		pos->get_candidate_num() != cand_to_change && pos->
		get_score() == highest_score; ++pos) {}

	if (pos != output.end() && pos->get_score() == highest_score) {
		return (false);    // has X top.
	}

	// Otherwise, just replace with plump!
	output.clear();
	output.insert(candscore(cand_to_change, 1));

	return (true);
}

///// Mono-sub-top
//////////////////

bool mono_sub_top::alter_ballot(const ordering & input, ordering & output,
	size_t numcands, const std::vector<size_t> & data,
	rng & randomizer) const {

	double highest_score = output.begin()->get_score();
	size_t cand_to_change = data[0];

	ordering::const_iterator pos;
	for (pos = output.begin(); pos != output.end() &&
		pos->get_candidate_num() != cand_to_change && pos->
		get_score() == highest_score; ++pos) {}

	if (pos != output.end() && pos->get_score() == highest_score) {
		return (false);    // has X top.
	}

	// Should have x top...
	output.clear();
	output.insert(candscore(cand_to_change, 10));

	// ... and then random after that.
	// Again, handle truncation (and ER) later. (Perhaps pass random
	// generator to function?)
	for (size_t counter = 0; counter < numcands; ++counter)
		if (counter != cand_to_change) {
			output.insert(candscore(counter, randomizer.next_double()));
		}

	return (true);
}
