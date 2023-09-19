// Random candidate. Simply take the available hopeful candidates, shuffle them,
// and output in order. It's obviously not cloneproof.

#include "../method.h"
#include "../../tools/tools.h"
#include "../../ballots.h"

#include "randcand.h"


std::pair<ordering, bool> random_candidate::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	std::vector<int> candidates;
	candidates.reserve(num_candidates);

	ordering toRet;

	size_t counter;

	for (counter = 0; counter < hopefuls.size(); ++counter)
		if (hopefuls[counter]) {
			candidates.push_back(counter);
		}

	random_shuffle(candidates.begin(), candidates.end());

	for (counter = 0; counter < candidates.size(); ++counter) {
		toRet.insert(candscore(candidates[counter], counter));
	}

	return (std::pair<ordering, bool>(toRet, false));
}

