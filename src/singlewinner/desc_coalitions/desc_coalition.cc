#include "desc_coalition.h"
#include <vector>

void desc_coalition_method::sort_by_candidate(
	std::vector<coalition_entry> & coalitions, int candidate) const {

	for (size_t i = 0; i < coalitions.size(); ++i) {
		coalitions[i].priority_candidate = i;
	}

	std::sort(coalitions.begin(), coalitions.end(),
		std::greater<coalition_entry>());
}

bool desc_coalition_method::can_candidate_win(
		std::vector<coalition_entry> & coalitions,
		const std::set<int> & starting_candidate_set,
		int candidate, int num_candidates) const {

	sort_by_candidate(coalitions, candidate);

	std::set<int> coalition_so_far = starting_candidate_set;

	for (std::vector<coalition_entry>::const_iterator pos = coalitions.begin();
			pos != coalitions.end(); ++pos) {

		std::set<int> test_intersection;

		std::set_intersection(coalition_so_far.begin(), coalition_so_far.end(),
			pos->coalition.begin(), pos->coalition.end(),
			std::inserter(test_intersection, test_intersection.end()));

		// If it's empty, skip.
		if (test_intersection.size() == 0) {
			continue;
		}

		// If there's only one candidate left, return true if that candidate
		// is the specified candidate, false otherwise.
		if (test_intersection.size() == 1) {
			return(test_intersection.find(candidate) != 
				test_intersection.end());
		}

		coalition_so_far = test_intersection;
	}

	return(coalition_so_far.find(candidate) != coalition_so_far.end());
}

std::pair<ordering, bool> desc_coalition_method::elect_inner(
		const std::list<ballot_group> & papers,
		const std::vector<bool> & hopefuls, int num_candidates, 
		cache_map * cache, bool winner_only) const {

	// Performance enhancement is possible: traditional DSC algorithm if 
	// winner_only is true.

	// Get the coalitions corresponding to this ballot group.

	std::map<std::set<int>, double> coalition_count;
	std::set<int> current_coalition, all_candidates;

	for (int i = 0; i < num_candidates; ++i) {
		if (hopefuls[i]) {
			all_candidates.insert(i);
		}
	}

	std::vector<coalition_entry> coalitions = get_coalitions(papers, 
		hopefuls, num_candidates);

	ordering outcome;

	// To decide the full ordering, we check what candidates can be made to
	// win by sorting the coalitions so that coalitions with that candidate
	// break ties between equal-support coalitions. All candidates who can
	// are then ranked at the current rank. We then decrease the rank,
	// eliminate the candidates we've determined the rank of from the initial
	// set, and redo the procedure to get what candidates are at next rank,
	// and so on.

	// Unknown_candidates are the candidates we don't know the rank of yet.
	// Revealed_candidates are the candidates we've revealed during that
	// turn.
	std::set<int> unknown_candidates = all_candidates,
					revealed_candidates;
	int rank_score = num_candidates;

	std::set<int>::const_iterator pos;

	while (!unknown_candidates.empty()) {
		revealed_candidates.empty();
		for (pos = unknown_candidates.begin(); pos != 
				unknown_candidates.end(); ++pos) {

			if (can_candidate_win(coalitions, unknown_candidates, *pos,
					num_candidates)) {

				outcome.insert(candscore(*pos, rank_score));
				revealed_candidates.insert(*pos);
			}
		}

		for (pos = revealed_candidates.begin(); 
				pos != revealed_candidates.end(); ++pos) {
			unknown_candidates.erase(*pos);
		}

		--rank_score;

		// If we only want the winners, we're almost done. Just add the
		// non-winners as equal last.
		if (winner_only) {

			for (pos = unknown_candidates.begin(); pos != 
					unknown_candidates.end(); ++pos) {
				outcome.insert(candscore(*pos, rank_score));
			}

			return(std::pair<ordering, bool>(outcome, winner_only));
		}
	}

	return(std::pair<ordering, bool>(outcome, winner_only));
}
