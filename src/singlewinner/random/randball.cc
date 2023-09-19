// Random ballot (or "random dictator"). The method is strategy-proof.

#include "../method.h"
#include "../../tools/tools.h"
#include "../../ballots.h"

#include "randball.h"


std::pair<ordering, bool> random_ballot::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// We need two rounds through, first one to determine the number of
	// voters, and then once to find which ballot to pick. After finding
	// n, the number of voters, we then determine k on 0..n and stop
	// when our running sum is >= k. This is akin to roulette selection
	// in genetic algorithms.

	double num_voters = 0;
	election_t::const_iterator pos;
	ordering::const_iterator opos;

	for (pos = papers.begin(); pos != papers.end(); ++pos) {
		num_voters += pos->get_weight();
	}

	double threshold = drand48() * num_voters, running_sum = 0;

	ordering prelim;

	// Here we have a magic constant to get around the possible unlucky
	// case that only few voters vote for the hopefuls. We could do it
	// more rigorously by shuffling the list, but there's no need most
	// of the time.
	// We could also implement random voter hierarchy instead...

	size_t retries = 3;
	size_t counter;

	for (counter = 0; counter < retries && prelim.empty(); ++counter) {

		for (pos = papers.begin(); pos != papers.end() &&
			prelim.empty(); ++pos) {

			running_sum += pos->get_weight();

			if (running_sum >= threshold) {
				// Add all hopefuls. If the voter only ranks
				// non-hopefuls, the !empty loop condition will
				// make the loop proceed as if that ballot
				// didn't exist.
				for (opos = pos->contents.begin(); opos !=
					pos->contents.end(); ++opos)
					if (hopefuls[opos->get_candidate_num()]) {
						prelim.insert(*opos);
					}
			}
		}
	}

	// If it's still empty, bring out the hard tools: make a copy of the
	// ballots and reshuffle them, then go through from beginning to end
	// until we get someting.

	if (prelim.empty()) {
		std::vector<ballot_group> rnd_papers;
		copy(papers.begin(), papers.end(), back_inserter(rnd_papers));
		random_shuffle(rnd_papers.begin(), rnd_papers.end());

		for (std::vector<ballot_group>::const_iterator vpos =
				rnd_papers.begin(); vpos != rnd_papers.end() &&
			prelim.empty(); ++vpos) {

			for (opos = pos->contents.begin(); opos !=
				pos->contents.end(); ++opos)
				if (hopefuls[opos->get_candidate_num()]) {
					prelim.insert(*opos);
				}
		}
	}

	// Now we have a preliminary ordering. It might be truncated, so add
	// all others in random order below those that are already present.
	// If nobody has a preference, we get random candidate.

	std::vector<bool> already_seen(num_candidates, false);

	for (opos = prelim.begin(); opos != prelim.end(); ++opos) {
		already_seen[opos->get_candidate_num()] = true;
	}

	double least_score = 1;
	if (!prelim.empty()) {
		least_score = prelim.rbegin()->get_score();
	}

	for (counter = 0; counter < already_seen.size(); ++counter)
		if (hopefuls[counter] && !already_seen[counter])
			// Add with a random score lower than the least ranked.
			prelim.insert(candscore(counter, least_score -
					fabs(least_score) * drand48()));

	return (std::pair<ordering, bool>(prelim, false));
}
