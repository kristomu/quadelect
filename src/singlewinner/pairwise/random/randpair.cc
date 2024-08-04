// Random Pair. This random method picks two candidates at random and elects
// the winner of the two. If they're tied, the method simply flips a coin.

#include "randpair.h"
#include "hack/msvc_random.h"

#include <assert.h>
#include <iostream>
#include <list>

std::pair<ordering, bool> randpair::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	bool debug = false;

	// Here we keep a list of hopeful candidates. As a candidate is
	// added to the output ordering, it is moved to the beginning and
	// the num_handled counter is incremented. Thus the first pair is
	// drawn randomly from [0..n], the next pair from [1..n] and so on.
	std::vector<int> candidates;
	size_t num_handled = 0;

	size_t counter;
	for (counter = 0; counter < hopefuls.size(); ++counter)
		if (hopefuls[counter]) {
			candidates.push_back(counter);
		}

	ordering toRet;
	int first, second, left, winner;

	while (num_handled < candidates.size()) {
		left = candidates.size() - num_handled;
		// If there's just a single candidate left, admit it.
		if (left == 1) {
			winner = num_handled;
		} else {
			// If not, we have to find the pair randomly.
			first = num_handled + (random() % left);
			do {
				second = num_handled + (random() % left);
			} while (first == second);

			// Then the winner is, well, whoever wins. If equal,
			// just do a coin flip, here handled by giving the
			// victory to the first; since both are picked randomly,
			// that is unbiased.
			if (debug)
				std::cout << "Checking " << candidates[first]
					<< " vs " << candidates[second] << std::endl;

			if (input.get_magnitude(candidates[first],
					candidates[second], hopefuls) >=
				input.get_magnitude(candidates[second],
					candidates[first],
					hopefuls)) {
				winner = first;
			} else	{
				winner = second;
			}
		}

		// Admit our winner to the ordering...
		if (debug)
			std::cout << "Winner is " << candidates[winner]
				<< " and gets " << left << " points." << std::endl;

		toRet.insert(candscore(candidates[winner], left));
		// and move it out of the way.
		std::swap(candidates[winner], candidates[num_handled]);
		++num_handled;

		// If we only wanted the winner, add the rest tied last
		// and get outta here.
		if (winner_only) {
			for (counter = num_handled; counter < candidates.size();
				++counter) {
				toRet.insert(candscore(candidates[counter], 0));
			}

			if (debug) {
				std::cout << "All done(TRUE)" << std::endl;
			}

			return (std::pair<ordering, bool>(toRet, true));
		}
	}

	if (debug) {
		std::cout << "All done(FALSE)" << std::endl;
	}

	return (std::pair<ordering, bool>(toRet, false));
}

