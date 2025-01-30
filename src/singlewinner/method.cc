#include "common/ballots.h"
#include "tools/tools.h"
#include "common/cache.h"
#include <iostream>
#include <vector>
#include <list>

#include "method.h"


// The default way of electing if we only have the "with hopefuls" method
// implemented is to call it with every candidate being a hopeful, so do that
// here. If the method in question wishes, it can override the function with a
// quicker one - that's why it's virtual.
std::pair<ordering, bool> election_method::elect_inner(
	const election_t & papers, int num_candidates,
	cache_map * cache, bool winner_only) const {

	std::vector<bool> hopefuls(num_candidates, true);

	return (elect_inner(papers, hopefuls, num_candidates, cache,
				winner_only));
}

// Cache wrappers.

std::pair<ordering, bool> election_method::elect_detailed(
	const election_t & papers, int num_candidates,
	cache_map * cache, bool winner_only) const {

	if (num_candidates == 0) {
		throw std::invalid_argument("Error: Can't call election with "
			"no candidates!");
	}

	// If there are only two candidates, then winner_only and full ordering
	// is the same, so set winner_only to true for a speedup. (It might also
	// be the same in case of (n-1)-way loser ties for n > 2, but we can't
	// detect that in a general case.)

	if (num_candidates <= 2) {
		winner_only = true;
	}

	// If we have a cache, try to look up the answer. We have to do it
	// this way because has_outcome wastes another name() call or in some
	// other way becomes too slow. Let's hear it for eager evaluation!
	std::pair<ordering, bool> toRet;

	if (cache != NULL) {
		toRet = cache->get_outcome(name(), winner_only);

		// If we got something, then return it...
		if (!toRet.first.empty()) {
			return (toRet);
		}
	}

	// Otherwise, get the output the hard way.
	toRet = elect_inner(papers, num_candidates, cache,
			winner_only);

	// Check that there are no errors.
	assert(toRet.first.size() == (size_t)num_candidates);

	// There were none, so set the cache...
	if (cache != NULL) {
		//std::cout << "Setting cache for " << name() << std::endl;
		cache->set_outcome(name(), toRet);
	}

	// ... and return the value.
	return (toRet);
}

std::pair<ordering, bool> election_method::elect_detailed(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	if (num_candidates == 0) {
		throw std::invalid_argument("Error: Can't call election with "
			"no candidates!");
	}

	if (hopefuls.size() != (size_t)num_candidates) {
		throw std::invalid_argument("Error: The list of hopefuls doesn't"
			"match the number of candidates!");
	}

	// ... a little optimization...
	if (num_candidates <= 2) {
		winner_only = true;
	}

	// If all are hopeful, it's just an ordinary election and so we can
	// make use of the cache. Detect that.

	int num_hopefuls = 0;
	std::vector<bool>::const_iterator hpos;

	for (hpos = hopefuls.begin(); hpos != hopefuls.end(); ++hpos) {
		if (*hpos) {
			++num_hopefuls;
		}
	}

	// There used to be a check that would return (last hopeful, 1) here
	// if there was only one hopeful candidate left. However, we can't do
	// that for rated or weighted positional systems, where the number of
	// points of the sole winner matters. This has thus been removed.
	// Fix the special case elsewhere!



	// If every candidate is in play, consult the cache if possible.

	if (num_hopefuls == num_candidates) {
		return (elect_detailed(papers, num_candidates, cache,
					winner_only));
	}

	if (num_hopefuls == 0) {
		throw std::invalid_argument("Error: Can't call election with "
			"no candidates!");
	}

	// If only one candidate is in play, return only that candidate.
	if (num_hopefuls == 1) {
		// Only one viable candidate.
		ordering hopeful;
		for (size_t cand = 0; cand < hopefuls.size(); ++cand) {
			if (hopefuls[cand]) {
				hopeful.insert(candscore(cand, 1));
			}
		}
		return std::pair<ordering, bool>(hopeful, false);
	}

	// Otherwise, go about it the hard way.

	std::pair<ordering, bool> toRet = elect_inner(papers, hopefuls,
			num_candidates, cache, winner_only);

	// Check that there are no errors.
	// BLUESKY: make num_* size_t.
	assert(toRet.first.size() == (size_t)num_hopefuls);

	// Then return! We can't use cache as there could be collisions
	// between different hopefuls patterns.
	// Bluesky: build a prefix according to the hopefuls? E.g. IRV
	// round with A and B excluded, C and D remaining becomes
	// __##/Plurality... Is it worth it? Find out when we have a tester,
	// but probably not.
	return (toRet);
}

// Public wrappers.
ordering election_method::elect(const election_t & papers,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	return (elect_detailed(papers, num_candidates, cache, winner_only).
			first);
}

ordering election_method::elect(const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	return (elect_detailed(papers, hopefuls, num_candidates, cache,
				winner_only).first);
}
