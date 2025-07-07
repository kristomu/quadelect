
#include "singlewinner/dmt/resistant/subelections.h"
#include "scw.h"

#include <iostream>

std::pair<ordering, bool> super_condorcet_set::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	size_t numcands = hopefuls.size();

	subelections se;
	se.count_subelections(papers, hopefuls, true);

	disqual_tensor level_disqualifications =
		se.get_level_disqualifications(hopefuls, true);

	size_t super_cw_idx = 0;
	bool found_super_condorcet = false;

	std::vector<size_t> copeland_counts(numcands, 0);

	for (size_t i = 0;
		i < numcands && !found_super_condorcet; ++i) {
		bool is_super_cw = true;

		for (size_t j = 0;
			j < numcands /*&& is_super_cw*/; ++j) {

			if (!hopefuls[i]) {
				continue;
			}
			if (!hopefuls[j]) {
				continue;
			}
			if (i == j) {
				continue;
			}

			if (!level_disqualifications[numcands][i][j]) {
				is_super_cw = false;
			} else {
				++copeland_counts[i];
			}
		}

		if (is_super_cw) {
			found_super_condorcet = true;
			super_cw_idx = i;
		}
	}

	ordering output;

	for (size_t cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		if (found_super_condorcet && super_cw_idx == cand) {
			output.insert(candscore(cand, 1));
		} else {
			output.insert(candscore(cand, 0));
		}
	}

	return std::pair<ordering, bool>(output, false);
}
