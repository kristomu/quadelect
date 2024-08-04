#include "mutual_majority.h"
#include "coalitions/coalitions.h"

std::pair<ordering, bool> mutual_majority_set::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	std::vector<coalition_data> solid_coalitions =
		get_solid_coalitions(papers, hopefuls,
			num_candidates);

	// Now find the smallest set supported by a majority,
	// and return.
	double numvoters = 0;
	for (const ballot_group & paper: papers) {
		numvoters += paper.get_weight();
	}

	coalition_data current_best = *solid_coalitions.begin();
	bool found = false;
	for (const coalition_data & cur_coalition: solid_coalitions) {
		if (cur_coalition.score > numvoters/2) {
			if (!found || cur_coalition.coalition.size() <
				current_best.coalition.size()) {
				current_best = cur_coalition;
				found = true;
			}
		}
	}

	// If we didn't find a single candidate mutual majority set,
	// something is *wrong*.
	assert(found);

	std::vector<bool> in_set(num_candidates, false);
	for (int candidate_idx: current_best.coalition) {
		in_set[candidate_idx] = true;
	}

	ordering mm_set;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		if (in_set[cand]) {
			mm_set.insert(candscore(cand, 1));
		} else {
			mm_set.insert(candscore(cand, 0));
		}
	}

	return std::pair<ordering, bool>(mm_set, false);
}