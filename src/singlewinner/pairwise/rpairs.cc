// Ranked Pairs / MAM.

// Ranked Pairs:

// TIDEMAN, T. Nicolaus. Independence of clones as a criterion for voting
// rules. Social Choice and Welfare, 1987, 4.3: 185-206.

// MAM:		http://alumnus.caltech.edu/~seppley/
// River:	http://lists.electorama.com/pipermail/
//				election-methods-electorama.com/2004-October/014018.html

#include "pairwise/matrix.h"
#include "method.h"
#include "../method.h"

#include <assert.h>
#include <iostream>
#include <list>

#include "rpairs.h"


// TODO? Return number of steps required or -1 if not there. Then if # steps
// == num candidates, we can bail early.
bool ranked_pairs::st_connectivity(int source, int dest,
	const std::vector<std::list<int> > & adjacency_lists) const {

	// If we're at the destination, all done. Otherwise recurse into every
	// node connected to ours. Note: if the graph has a cycle, we'll be
	// stuck. We could get around that by making record of where we've
	// been, but I don't see a need for that yet.

	if (source == dest) {
		return (true);
	}

	for (std::list<int>::const_iterator pos = adjacency_lists[source].begin();
		pos != adjacency_lists[source].end(); ++pos)
		if (st_connectivity(*pos, dest, adjacency_lists)) {
			return (true);
		}

	return (false);
}

// A breadth-first traversal might be quicker, but would also be more complex.
void ranked_pairs::traverse_tree(std::vector<int> & places, int node,
	int depth,
	const std::vector<std::list<int> > & adjacency_lists) const {

	places[node] = std::max(places[node], depth);

	for (std::list<int>::const_iterator pos = adjacency_lists[node].begin();
		pos != adjacency_lists[node].end(); ++pos) {
		traverse_tree(places, *pos, depth + 1, adjacency_lists);
	}
}

std::string ranked_pairs::pw_name() const {

	if (is_river) {
		return ("River");
	} else	{
		return ("Ranked Pairs");
	}
}

std::pair<ordering, bool> ranked_pairs::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	// First dump the n^2 - n different contests into a vector.

	std::vector<beat_component> contests;
	int numcand = input.get_num_candidates();
	contests.reserve(numcand * (numcand - 1));

	size_t counter, sec;

	for (counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}
		for (sec = 0; sec < input.get_num_candidates(); ++sec) {
			if (counter == sec || !hopefuls[sec]) {
				continue;
			}

			contests.push_back(beat_component(counter, sec,
					input.get_magnitude(counter,
						sec, hopefuls)));
		}
	}

	// Then sort them so greatest magnitude comes first.

	sort(contests.begin(), contests.end(), std::greater<beat_component>());

	// For each contest: check if it would induce a cycle (or would cause
	// a branching, if River). If not, add it to the adjacency lists.

	std::vector<std::list<int> > adjacency_lists(numcand);
	std::vector<bool> can_win(numcand, true);

	int num_admitted = 0;
	int max_admissible = (numcand * (numcand + 1))/2;

	for (std::vector<beat_component>::const_iterator pos = contests.begin();
		pos != contests.end() && num_admitted <= max_admissible;
		++pos) {
		if ((!is_river || can_win[pos->loser]) &&
			!st_connectivity(pos->loser, pos->winner,
				adjacency_lists)) {
			adjacency_lists[pos->winner].push_back(pos->loser);
			can_win[pos->loser] = false;
			++num_admitted;
		}
	}

	// Find the winner. If it's winner-only, all we have to do is dump
	// the winner into our output ordering; if not, we also have to run a
	// tree traversal to update the ranks of the rest. (It's relatively
	// easy to see there can be only one winner.)

	ordering out;

	if (winner_only) {
		for (counter = 0; counter < (size_t)numcand; ++counter)
			if (can_win[counter]) {
				out.insert(candscore(counter, 1));
			} else	{
				out.insert(candscore(counter, 0));
			}

		return (std::pair<ordering, bool>(out, true));
	}

	// If we want a full ordering, we need to traverse the tree to get the
	// rank.

	std::vector<int> places(numcand, -1);
	int idzero = -1;

	for (counter = 0; counter < (size_t)numcand && idzero == -1; ++counter)
		if (can_win[counter]) {
			idzero = counter;
		}

	traverse_tree(places, idzero, 0, adjacency_lists);

	for (counter = 0; counter < places.size(); ++counter) {
		out.insert(candscore(counter, -places[counter]));
	}

	return (std::pair<ordering, bool>(out, false));
}

