// Random candidate. Simply take the available hopeful candidates, shuffle them,
// and output in order. It's obviously not cloneproof.

#include "../method.h"
#include "../../tools/tools.h"
#include "../../ballots.h"

#include "randcand.h"

using namespace std;

pair<ordering, bool> random_candidate::elect_inner(
	const list<ballot_group> & papers,
	const vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	vector<int> candidates;
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

	return (pair<ordering, bool>(toRet, false));
}

