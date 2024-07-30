#include "coalitions.h"

#include <iterator>

// TODO: Update comments and combine DAC and DSC functions to make HDSC.
// The DAC function really needs some OPTIMIZATION!

std::vector<coalition_data> get_solid_coalitions(
	const election_t & elections,
	const std::vector<bool> & hopefuls, int numcands) {

	// Go through the ballot set, incrementing the coalition counts.

	std::map<std::set<int>, double> coalition_count;
	std::set<int> current_coalition, all_candidates;

	for (int i = 0; i < numcands; ++i) {
		if (hopefuls[i]) {
			all_candidates.insert(i);
		}
	}

	for (election_t::const_iterator ballot = elections.begin();
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

	return coalitions;
}

// Descending Acquiescing Coalitions helper. This can probably be
// optimized further (later).

// The general idea is that for each ballot we create a list of level
// sets: candidates ranked equal by that voter. Then for each level
// set, we add one to every coalition consisting of every candidate in
// higher-ranked level sets, in addition to a subset of the/ current
// level set, recursively.

// Recursive helper for adding every subset of the current level set.
void add_acquiescing_coalition(
	double ballot_weight,
	const std::vector<int> & current_level_set,
	size_t position, size_t level_set_size,
	size_t included_elements, std::set<int> & current_coalition,
	std::map<std::set<int>, double> & coalition_count) {


	// We've gone through the whole level set, get ready
	// to increment the coalition count.
	if (position == level_set_size) {
		// But only if we added at least one element from the
		// current level set.
		if (included_elements > 0) {
			coalition_count[current_coalition] += ballot_weight;
		}
		return;
	}

	// Pick the current level set member, recurse, then remove it
	// and recurse again.
	current_coalition.insert(current_level_set[position]);
	add_acquiescing_coalition(ballot_weight, current_level_set,
		position+1, level_set_size, included_elements+1,
		current_coalition, coalition_count);
	current_coalition.erase(current_level_set[position]);
	add_acquiescing_coalition(ballot_weight, current_level_set,
		position+1, level_set_size, included_elements,
		current_coalition, coalition_count);
}

std::vector<coalition_data> get_acquiescing_coalitions(
	const election_t & elections,
	const std::vector<bool> & hopefuls, int numcands) {

	std::map<std::set<int>, double> coalition_count;

	std::set<int> all_candidates;

	for (int i = 0; i < numcands; ++i) {
		if (hopefuls[i]) {
			all_candidates.insert(i);
		}
	}

	for (election_t::const_iterator ballot = elections.begin();
		ballot != elections.end(); ++ballot) {

		// Create the level sets.
		std::vector<std::vector<int> > level_sets;
		std::vector<int> current_level_set;
		std::set<int> remaining_candidates = all_candidates;

		ordering::const_iterator pos = ballot->contents.begin(),
								 level_start = pos;

		while (level_start != ballot->contents.end()) {
			pos = level_start;
			while (pos != ballot->contents.end() &&
				pos->get_score() == level_start->get_score()) {
				if (hopefuls[pos->get_candidate_num()]) {
					current_level_set.push_back(pos->get_candidate_num());
					remaining_candidates.erase(pos->get_candidate_num());
				}
				++pos;
			}
			// Now we're either at the end or the candidate
			// no longer has the score that every candidate in
			// the current level set has got. In either case, the
			// current level set is done - get ready to start a new one.

			// An empty set can happen if there were no hopefuls at this
			// level. If so, ignore it.
			if (!current_level_set.empty()) {
				level_sets.push_back(current_level_set);
			}
			current_level_set.clear();
			level_start = pos;
		}

		// Insert the remaining candidates set as the final level set
		// if it's nonempty. This handles truncation.
		if (!remaining_candidates.empty()) {
			level_sets.push_back(std::vector<int>(remaining_candidates.begin(),
					remaining_candidates.end()));
		}

		// For each level set: process that level set, then add the whole
		// set into the current_coalition set, as every higher level set
		// needs to be included in later acquiescing coalitions.

		std::set<int> current_coalition;

		for (const std::vector<int> & level_set: level_sets) {
			add_acquiescing_coalition(ballot->get_weight(),
				level_set, 0, level_set.size(), 0, current_coalition,
				coalition_count);
			std::copy(level_set.begin(), level_set.end(), std::inserter(
					current_coalition, current_coalition.begin()));
		}
	}

	std::vector<coalition_data> coalitions;
	for (std::map<std::set<int>, double>::const_iterator
		pos = coalition_count.begin(); pos != coalition_count.end();
		++pos) {
		coalitions.push_back(coalition_data(pos->second, pos->first));
	}

	return coalitions;
}
