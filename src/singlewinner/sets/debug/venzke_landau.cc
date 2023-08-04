#include "venzke_landau.h"

std::pair<ordering, bool> venzke_landau_set::pair_elect(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	size_t num_candidates = input.get_num_candidates();

	std::vector<bool> covered(num_candidates, false);

	for (size_t x = 0; x < num_candidates; ++x) {
		if (!hopefuls[x]) {
			continue;
		}
		for (size_t z = 0; z < num_candidates; ++z) {
			if (!hopefuls[z]) {
				continue;
			}
			if (x == z) {
				continue;
			}

			bool found_y = false;
			for (size_t y = 0; y < num_candidates && !found_y; ++y) {
				if (!hopefuls[y]) {
					continue;
				}
				if (input.beats_or_ties(x, y) && input.beats_or_ties(y, z)) {
					found_y = true;
				}
			}

			// If found Y, then Z can't be covering X; otherwise X is
			// covered by Z.
			if (!found_y) {
				covered[x] = true;
			}
		}
	}

	ordering landau_set;

	for (size_t candidate = 0; candidate < num_candidates; ++candidate) {
		if (!hopefuls[candidate]) {
			continue;
		}

		if (covered[candidate]) {
			landau_set.insert(candscore(candidate, 0));
		} else {
			landau_set.insert(candscore(candidate, 1));
		}
	}

	return (std::pair<ordering, bool>(landau_set, false));
}
