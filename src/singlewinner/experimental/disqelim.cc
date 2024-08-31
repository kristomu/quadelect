#include "disqelim.h"

typedef ptrdiff_t ssize_t;	/* ssize_t is not part of the C standard */

// Helper function, put elsewhere in due order
int count_trues(const std::vector<bool> & x) {
	int count = 0;
	for (bool b: x) {
		if (b) {
			++count;
		}
	}

	return count;
}

bool disqelim::disqualifies(int candidate,
	const election_t & papers,
	const std::vector<bool> & continuing,
	const std::vector<bool> & hopefuls,
	int num_candidates) const {

	//std::cout << "\tDisqelim: checking " << candidate << std::endl;

	std::vector<bool> disqualified = hopefuls;

	std::vector<std::vector<bool > > contd_power_set =
		power_set(continuing);

	disqualified[candidate] = false;

	// For every subset of the continuing set:
	//	if the candidate is not in it, skip.
	//	Otherwise, get the Plurality count restricted to this set.
	//	If candidate > 1/n, no problem, go to the next one.
	//	Otherwise, for every other candidate i
	//		disqualified[i] = false.

	// Count the number of disqualified candidates. Return whether
	// it's > 0.

	// TODO? Do all candidates at once??? E.g. return a v<bool>.
	// Sounds like a better idea, later.

	for (const std::vector<bool> & candidate_selection:
		contd_power_set) {

		if (!candidate_selection[candidate]) {
			continue;
		}

		// Extracted from rmr1.cc

		int num_remaining_candidates_here = 0;

		for (size_t i = 0; i < candidate_selection.size(); ++i) {
			if (candidate_selection[i]) {
				++num_remaining_candidates_here;
			}
		}

		ordering plur_outcome;

		if (num_remaining_candidates_here > 0) {
			plur_outcome = plurality_method.elect(papers,
					candidate_selection, num_candidates, NULL, false);
		}

		std::vector<double> first_prefs_this_selection(
			num_candidates, 0);
		double num_remaining_voters_here = 0;

		for (const candscore & cand_and_score: plur_outcome) {
			assert(cand_and_score.get_candidate_num() < (size_t)num_candidates);
			first_prefs_this_selection[cand_and_score.get_candidate_num()] =
				cand_and_score.get_score();
			num_remaining_voters_here += cand_and_score.get_score();
		}

		// (Cut and paste code ends)

		// If we're above the threshold, no problem.

		if (num_remaining_candidates_here *
			first_prefs_this_selection[candidate] >
			num_remaining_voters_here) {
			continue;
		}

		// Otherwise cancel every disqualification of someone
		// else in this set by us.

		for (size_t saved_cand = 0; saved_cand < candidate_selection.size();
			++saved_cand) {

			// If the candidate is not in the selection, skip
			if (!candidate_selection[saved_cand]) {
				continue;
			}

			// If the candidate isn't disqualified, no need to
			// update.
			if (!disqualified[saved_cand]) {
				continue;
			}

			disqualified[saved_cand] = false;

			// TODO: Return early if we don't disqualify
			// anyone; no need to test further.
		}
	}

	for (size_t i = 0; i < disqualified.size(); ++i) {
		if (!continuing[i]) {
			continue;
		}

		if (disqualified[i]) {
			//std::cout << "\tWe disqualified " << i << std::endl;
			return true;
		}
	}

	return false;
}

std::pair<ordering, bool> disqelim::elect_inner(
	const election_t & papers, const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	std::vector<int> scores(num_candidates, -num_candidates);
	size_t remaining_candidates = count_trues(hopefuls);

	// Starting with the full complement: travel from the end of the
	// base ordering until we find someone who doesn't disqualify
	// anyone else. Eliminate him and redo. The output order is in
	// reverse order of the elimination.

	ordering remaining_order;

	// If we have a base method, use it. WARNING: Violates neutrality
	// if the base method returns a tie. Properly speaking the tie
	// should be broken, e.g. by random ballot, but I can't be arsed.

	if (has_method) {
		remaining_order = base_method->elect(papers,
				hopefuls, num_candidates, cache, false);
	} else {
		remaining_order	 = base_ordering;
	}

	std::vector<bool> continuing = hopefuls;

	// Hack: if the base order is the wrong length, make a new one.
	// Note that this violates neutrality!

	int cand;

	// NOTE: Some serious type confusion here. With very large
	// numbers of candidates, this could be a problem... there are
	// lots of bugs of this type lurking around in general.

	if (remaining_order.size() != remaining_candidates) {
		remaining_order.clear();
		for (cand = 0; cand < num_candidates; ++cand) {
			if (!hopefuls[cand]) {
				continue;
			}
			remaining_order.insert(candscore(cand, cand));
		}
	}

	while (remaining_candidates > 0) {
		auto next_to_eliminate = remaining_order.rbegin();
		while (disqualifies(next_to_eliminate->get_candidate_num(),
				papers, continuing, hopefuls, num_candidates)) {
			//std::cout << "Checked candidate " <<
			//	next_to_eliminate->get_candidate_num() << std::endl;
			++next_to_eliminate;
		}

		int to_eliminate = next_to_eliminate->get_candidate_num();
		//std::cout << "Yeeting " << to_eliminate << std::endl;

		continuing[to_eliminate] = false;
		scores[to_eliminate] = -(ssize_t)(remaining_candidates);
		--remaining_candidates;
		remaining_order.erase(*next_to_eliminate);
	}

	ordering output_order;

	for (int cand = 0; cand < num_candidates; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		output_order.insert(candscore(cand, scores[cand]));
	}

	return std::pair<ordering, bool>(output_order, false);
}
