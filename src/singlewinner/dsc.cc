#include "dsc.h"
#include <vector>

void dsc::sort_by_candidate(std::vector<coalition_entry> & coalitions, 
	int candidate) const {

	for (size_t i = 0; i < coalitions.size(); ++i) {
		coalitions[i].priority_candidate = i;
	}

	std::sort(coalitions.begin(), coalitions.end(),
		std::greater<coalition_entry>());
}

bool dsc::can_candidate_win(std::vector<coalition_entry> & coalitions,
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

std::pair<ordering, bool> dsc::elect_inner(
		const std::list<ballot_group> & papers,
		const std::vector<bool> & hopefuls, int num_candidates, 
		cache_map * cache, bool winner_only) const {

	// Performance enhancement is possible: traditional DSC algorithm if 
	// winner_only is true.

	// Go through the ballot set, incrementing the coalition counts.

	std::map<std::set<int>, double> coalition_count;
	std::set<int> current_coalition, all_candidates;

	for (int i = 0; i < num_candidates; ++i) {
		if (hopefuls[i]) {
			all_candidates.insert(i);
		}
	}

	for (std::list<ballot_group>::const_iterator ballot = papers.begin();
			ballot != papers.end(); ++ballot) {

		current_coalition.clear();

		ordering::const_iterator opos = ballot->contents.begin();

		while (opos != ballot->contents.end()) {
			// The invariant is that we start at something that is
			// strictly lower ranked than every candidate closer to
			// pos->contents.begin().

			double current_score = opos->get_score();

			for (; opos != ballot->contents.end() && 
					opos->get_score() == current_score; ++opos) {
				if (hopefuls[opos->get_candidate_num()]) {
					current_coalition.insert(opos->get_candidate_num());
				}
			}

			// If we're using DSC as a component of an elimination system,
			// all candidates of a certain rank level might be eliminated.
			// In that case, the current coalition here will be empty, and
			// we should not add anything. E.g. the ranking is A>B=C>D and
			// both B and C are eliminated.

			if (!current_coalition.empty()) {
				coalition_count[current_coalition] += ballot->weight;
			}
		}

		// If the ballot is truncated, add the coalition of all candidates
		// also.

		if (current_coalition.size() != (size_t)num_candidates) {
			coalition_count[all_candidates] += ballot->weight;
		}
	}

	// Convert the coalition counts into an array form that we can sort by 
	// support. The actual sorting happens inside get_candidate_score.

	std::vector<coalition_entry> coalitions;
	for (std::map<std::set<int>, double>::const_iterator 
			pos = coalition_count.begin(); pos != coalition_count.end(); 
			++pos) {
		coalitions.push_back(coalition_entry(pos->second, pos->first));
	}

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

		// if winner only goes here

		--rank_score;
	}

	return(std::pair<ordering, bool>(outcome, false));
}
