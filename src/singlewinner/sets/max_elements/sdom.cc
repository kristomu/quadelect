
#include "sdom.h"

// Perhaps turn into matrix?

bool sdom_set::strongly_dominates(size_t y, size_t x,
	const abstract_condmat & input,
	const std::vector<bool> & hopeful) const {

	// Determine if dominator beats dominated. If so, continue; if it's
	// the other way around, then get the negative of the result with the
	// candidates swapped, and if they tie, then that's 0.

	double beat = input.get_magnitude(y, x, hopeful) - input.
		get_magnitude(x, y, hopeful);

	/*if (beat == 0)
		return(0);
	if (beat < 0)
		return(-strongly_dominates(x, y, input, hopeful));*/

	if (beat <= 0) {
		return (0);
	}

	// For each other candidate Z,
	bool still_dominates = true;

	for (size_t z = 0; z < input.get_num_candidates() && still_dominates;
		++z) {
		if (!hopeful[z]) {
			continue;
		}
		// *other*
		if (z == x || z == y) {
			continue;
		}

		// if Z beats Y, Z beats X even more.
		if (input.get_magnitude(z, y, hopeful) > input.
			get_magnitude(y, z, hopeful))
			still_dominates &= (input.get_magnitude(z, x, hopeful) >
					input.get_magnitude(z, y, hopeful));

		if (!still_dominates) {
			continue;
		}

		// if Z beats X, Y beats X even more.
		if (input.get_magnitude(z, x, hopeful) > input.
			get_magnitude(x, z, hopeful))
			still_dominates &= (input.get_magnitude(y, x, hopeful) >
					input.get_magnitude(z, x, hopeful));

		if (!still_dominates) {
			continue;
		}

		// if X beats Z, Y beats Z even more.
		if (input.get_magnitude(x, z, hopeful) >
			input.get_magnitude(z, x, hopeful))
			still_dominates &= (input.get_magnitude(y, z, hopeful) >
					input.get_magnitude(x, z, hopeful));

		if (!still_dominates) {
			continue;
		}
		// if Y beats Z, Y beats X even more.
		if (input.get_magnitude(y, z, hopeful) >
			input.get_magnitude(z, y, hopeful))
			still_dominates &= (input.get_magnitude(y, x, hopeful) >
					input.get_magnitude(y, z, hopeful));
	}

	return still_dominates;
}

std::pair<ordering, bool> sdom_set::pair_elect(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	// Make the empty "strongly dominates" matrix.
	condmat sdom_matrix(input.get_num_candidates(), input.get_num_voters(),
		CM_PAIRWISE_OPP);
	sdom_matrix.zeroize();

	std::vector<int> dominated(input.get_num_candidates(), 0);

	// Fill it with the results.
	size_t counter, sec;

	//std::cout << "Row dominates col" << std::endl;
	for (counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}
		for (sec = 0; sec < input.get_num_candidates(); ++sec) {
			if (!hopefuls[sec]) {
				continue;
			}
			bool result = false;

			if (counter != sec) {
				result = strongly_dominates(counter, sec,
						input, hopefuls);
			}
			if (result) {
				++dominated[counter];
				sdom_matrix.add(counter, sec, result);
				assert(sdom_matrix.get_magnitude(counter, sec) == result);
			}
		}
	}

	ordering toRet;

	for (counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}
		toRet.insert(candscore(counter, -dominated[counter]));
	}

	return std::pair<ordering, bool>(toRet, false);

	// Return the Smith set for this matrix.
	//return(std::pair<ordering,bool>(nested_sets(sdom_matrix, hopefuls), false));
}
