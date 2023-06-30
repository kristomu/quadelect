#include "dsc.h"
#include <vector>

// TODO: Update comments, make two functions (so HDSC will be easy).
// Also note for DAC that we need to construct all subsets of the implicit
// equal-last candidates for truncated ballots.

std::vector<coalition_entry> dsc::get_coalitions(
	const std::list<ballot_group> & papers,
	const std::vector<bool> & hopefuls, int num_candidates) const {

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
				coalition_count[current_coalition] += ballot->get_weight();
			}
		}

		// If the ballot is truncated, add the coalition of all candidates
		// also.

		if (current_coalition.size() != (size_t)num_candidates) {
			coalition_count[all_candidates] += ballot->get_weight();
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

	return (coalitions);
}