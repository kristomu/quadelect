#include "takedown.h"
#include "../../pairwise/matrix.h"
#include <limits>

// Avert your eyes, all who enter here :-P

// This could definitely be optimized and cleaned up.

/* This file implements a variant of Forest Simmons' CTE method.
   His method is defined as follows:

1. Elect the undefeated (remaining) candidate when there is one ... else ...
	eliminate any candidate that defeats no other (remaining) candidate ...
       	and ...
2. List the remaining candidates in order of [the base method's evaluation].
3. Update the list by [Bubble] Sorting it pairwise.
4. Let P (for Pivot) be the candidate that has sunk to the bottom of the list.
5. Eliminate P along with any candidates defeated by P.
6. While more than one candidate remains, repeat steps 2 through 5.
7. Elect the uneliminated candidate.

This method does the same, except it repeats from step one. */

std::pair<ordering, bool> cte::elect_inner(const
	election_t & papers, const std::vector<bool> & input_hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	ordering output_order;

	std::vector<bool> hopefuls = input_hopefuls;
	std::set<int> hopeful_set;

	size_t i, j, numcands = num_candidates; // hack

	for (i = 0; i < numcands; ++i) {
		if (hopefuls[i]) {
			hopeful_set.insert(i);
		}
	}

	if (hopeful_set.empty()) {
		return {};
	}

	if (hopeful_set.size() == 1) {
		output_order.insert(candscore(*hopeful_set.begin(), 0));

		return {output_order, false};
	}

	// OPT: We can do this calculation once instead of once per
	// recursion. Fix later.
	condmat condorcet_matrix(CM_WV);
	condorcet_matrix.count_ballots(papers, num_candidates);

	size_t least_output_score = 0;

	// Check for a CW. If there is one, the CW goes first and
	// everybody else below.

	for (i = 0; i < numcands; ++i) {
		if (!hopefuls[i]) {
			continue;
		}
		bool is_CW = true;
		for (j = 0; j < numcands && is_CW; ++j) {
			if (!hopefuls[j] || i == j) {
				continue;
			}
			is_CW &= condorcet_matrix.beats(i, j);
		}

		if (is_CW) {
			for (j = 0; j < numcands; ++j) {
				if (!hopefuls[j]) {
					continue;
				}
				if (j == i) {
					output_order.insert(candscore(j, 1));
				} else {
					output_order.insert(candscore(j, 0));
				}
			}
			return {output_order, false};
		}
	}

	// Check for a Condorcet loser; eliminate if found.
	bool is_CL = false;
	size_t condorcet_loser;

	for (i = 0; i < numcands && !is_CL; ++i) {
		if (!hopefuls[i]) {
			continue;
		}

		is_CL = true;
		for (j = 0; j < numcands && is_CL; ++j) {
			if (!hopefuls[j] || i == j) {
				continue;
			}
			is_CL &= condorcet_matrix.beats(j, i);
		}
		if (is_CL) {
			condorcet_loser = i;
		}
	}

	if (is_CL) {
		// Add to the bottom of the return order and eliminate.
		output_order.insert(candscore(condorcet_loser, least_output_score++));
		hopefuls[condorcet_loser] = false;
		hopeful_set.erase(condorcet_loser);
	}

	// Get the base method's social ordering and turn it into a
	// scores array for sorting purposes. (Note, these scores may be
	// closer to Borda scores than real scores.)
	ordering base_order = base->elect(papers, hopefuls,
			num_candidates, cache, false);
	std::vector<double> base_scores(num_candidates,
		-std::numeric_limits<double>::infinity());
	std::vector<size_t> sort_order;

	for (const candscore & cs: base_order) {
		base_scores[cs.get_candidate_num()] =
			cs.get_score();
		sort_order.push_back(cs.get_candidate_num());
	}

	// Do a descending bubble sort by score.
	bool sorted = false;
	while (!sorted) {
		sorted = true;
		for (i = 1; i < sort_order.size(); ++i) {
			if (base_scores[sort_order[i-1]] <
				base_scores[sort_order[i]]) {
				std::swap(base_scores[sort_order[i-1]],
					base_scores[sort_order[i]]);
				sorted = false;
			}
		}
	}

	// Eliminate the candidate at the bottom plus everybody he
	// pairwise defeats.

	size_t loser = *sort_order.rbegin();

	output_order.insert(candscore(loser, least_output_score));
	hopefuls[loser] = false;
	hopeful_set.erase(loser);

	for (i = 0; i < numcands; ++i) {
		if (hopefuls[i] && condorcet_matrix.beats(loser, i)) {
			output_order.insert(candscore(i, least_output_score));
			hopefuls[i] = false;
			hopeful_set.erase(i);
		}
	}

	++least_output_score;

	// Recurse and get the partial order after our eliminations.
	ordering after_elim = elect_inner(papers, hopefuls,
			num_candidates, cache, false).first;

	// Incorporate into the current ordering (all ranked above what we've
	// got so far) then return.

	// Normalize the lowest ranked/rated of the after_elim ordering
	// to least_output_score.
	least_output_score -= after_elim.rbegin()->get_score();

	for (candscore cs: after_elim) {
		cs.set_score(cs.get_score() + least_output_score);
		output_order.insert(cs);
	}

	return {output_order, false};
}
