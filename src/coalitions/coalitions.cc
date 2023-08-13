#include "coalitions.h"

// TODO: Update comments, make a DAC function too (will be harder), and
// then combine them into HDSC.
// Also note for DAC that we need to construct all subsets of the implicit
// equal-last candidates for truncated ballots.

std::vector<coalition_data> get_solid_coalitions(
	const std::list<ballot_group> & elections,
	const std::vector<bool> & hopefuls, int numcands) {

	// Go through the ballot set, incrementing the coalition counts.

	std::map<std::set<int>, double> coalition_count;
	std::set<int> current_coalition, all_candidates;

	for (int i = 0; i < numcands; ++i) {
		if (hopefuls[i]) {
			all_candidates.insert(i);
		}
	}

	for (std::list<ballot_group>::const_iterator ballot = elections.begin();
		ballot != elections.end(); ++ballot) {

		current_coalition.clear();

		ordering::const_iterator opos = ballot->contents.begin();

		while (opos != ballot->contents.end()) {
			// The invariant is that we start at something that is
			// strictly lower ranked than every candidate closer to
			// pos->contents.begin().

			// Skip past any eliminated candidates.
			if (!hopefuls[opos->get_candidate_num()]) {
				++opos;
			}

			double current_score = opos->get_score();

			for (; opos != ballot->contents.end() &&
				opos->get_score() == current_score; ++opos) {
				if (hopefuls[opos->get_candidate_num()]) {
					current_coalition.insert(opos->get_candidate_num());
				}
			}

			coalition_count[current_coalition] += ballot->get_weight();
		}

		// If the ballot is truncated, add the coalition of all candidates
		// also.

		if (current_coalition.size() != (size_t)numcands) {
			coalition_count[all_candidates] += ballot->get_weight();
		}
	}

	// Convert the coalition counts into an array form that we can sort by
	// support. The actual sorting happens inside get_candidate_score.

	std::vector<coalition_data> coalitions;
	for (std::map<std::set<int>, double>::const_iterator
		pos = coalition_count.begin(); pos != coalition_count.end();
		++pos) {
		coalitions.push_back(coalition_data(pos->second, pos->first));
	}

	return (coalitions);
}