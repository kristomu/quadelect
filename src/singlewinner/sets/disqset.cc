#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "disqset.h"
#include "max_elements/smith.h"

// Recursively test the indirect disqualification condition for
// chain_members.rbegin() ~> leaf:
//	Call the members last and leaf.
//	Go through each subelection, given by the membership set S:
//		If root is not in S, but both last and leaf are in it:
//			If fp{last} <= 1/|S|, then return (last doesn't disqualify
//				leaf).
//		If some suffix of chain_members list, *and* leaf is in S,
//			If the sum of first prefs of the suffix set Q, does
//				not exceed |Q|/|S|, then return (last doesn't
//				disqualify leaf through the root).
//	If we have gone through every applicable subelection without
//		aborting, then:
//		Set disqualified[leaf], and add leaf to the chain.
//		Recurse for every non-chain member.
//		Remove leaf from the chain and return.

// The chain must start with the root.

void idisqualif_set::explore_paths(
	const std::vector<bool> & hopefuls,
	const subelections & se,
	std::vector<size_t> & chain_members,
	std::vector<bool> & chain_members_bool,
	size_t leaf, std::vector<bool> & disqualified) const {

	if (chain_members.empty()) {
		throw std::invalid_argument(
			"explore_paths: Chain must contain root!");
	}

	/*size_t root = chain_members[0],
		   last = *chain_members.rbegin();*/
	size_t num_subelections = se.hopeful_power_set.size();

	for (size_t se_idx = 0; se_idx < num_subelections; ++se_idx) {
		// If leaf isn't in it, then it's not interesting.
		if (!se.hopeful_power_set[se_idx][leaf]) {
			continue;
		}

		// Find the earliest chain_members candidate in the
		// subelection.
		bool found_candidate = false;
		size_t earliest_chain_idx = 0;

		for (size_t i = 0; i < chain_members.size() && !found_candidate; ++i) {
			if (se.hopeful_power_set[se_idx][chain_members[i]]) {
				earliest_chain_idx = i;
				found_candidate = true;
			}
		}

		// If we didn't find any, then the subelection doesn't contain
		// last either, and so is uninteresting.
		if (!found_candidate) {
			continue;
		}

		// Skip if not everybody in the suffix set Q is in the subelection.
		bool all_members = true;
		double combined_first_prefs = 0;
		size_t seen_candidates = 0;
		for (size_t chain_cand_idx = earliest_chain_idx;
			chain_cand_idx < chain_members.size(); ++chain_cand_idx) {

			size_t chain_cand = chain_members[chain_cand_idx];

			all_members &= se.hopeful_power_set[se_idx][chain_cand];
			combined_first_prefs += se.first_pref_scores[se_idx][chain_cand];
			++seen_candidates;

			if (!all_members) {
				continue;
			}
		}
		if (!all_members) {
			continue;
		}

		// If we fail to disqualify leaf through the suffix set, abort.
		// combined_first_prefs <= numvoters * |Q| / |S|
		// i.e. |S| * cfs <= numvoters * |Q|
		if (se.num_remaining_candidates[se_idx] *
			combined_first_prefs <=
			se.num_remaining_voters[se_idx] * seen_candidates) {
			// last doesn't disqualify leaf.
			return;
		}
	}

	// Last does disqualify leaf!

	// Set disqualification and add the leaf to the chain.
	disqualified[leaf] = true;
	chain_members.push_back(leaf);
	assert(!chain_members_bool[leaf]);
	chain_members_bool[leaf] = true;

	// Recurse into every hopeful non-chain member.
	for (size_t candidate_leaf = 0; candidate_leaf < hopefuls.size();
		++candidate_leaf) {

		if (!hopefuls[candidate_leaf] ||
			chain_members_bool[candidate_leaf]) {
			continue;
		}

		explore_paths(hopefuls, se, chain_members,
			chain_members_bool, candidate_leaf,
			disqualified);
	}

	// Remove the leaf member from the chain so that whoever is
	// calling us can try another branch.
	chain_members.pop_back();
	assert(chain_members_bool[leaf]);
	chain_members_bool[leaf] = false;
}

std::vector<std::vector<bool> > idisqualif_set::explore_all_paths(
	const election_t & papers,
	const std::vector<bool> & hopefuls) const {

	size_t numcands = hopefuls.size();

	subelections se;
	se.count_subelections(papers, hopefuls, false);

	std::vector<std::vector<bool> > disqualification_matrix(
		numcands, std::vector<bool>(numcands, false));

	for (size_t root = 0; root < numcands; ++root) {
		if (!hopefuls[root]) {
			continue;
		}
		for (size_t leaf = 0; leaf < numcands; ++leaf) {
			if (!hopefuls[leaf] || root == leaf) {
				continue;
			}

			std::vector<size_t> chain_members = {root};
			std::vector<bool> chain_members_bool(numcands, false);
			chain_members_bool[root] = true;

			explore_paths(hopefuls, se, chain_members,
				chain_members_bool, leaf,
				disqualification_matrix[root]);
		}
	}

	return disqualification_matrix;
}

// Standard DFS search to detect a cycle.

bool idisqualif_set::has_cycle(
	const std::vector<std::vector<bool> > &	disqualifications,
	std::vector<bool> & visited,
	size_t cur_cand) const {

	size_t numcands = disqualifications.size();

	visited[cur_cand] = true;

	for (size_t next_candidate = 0; next_candidate < numcands;
		++next_candidate) {
		// Skip if there's no edge to next_candidate.
		if (!disqualifications[cur_cand][next_candidate]) {
			continue;
		}

		if (visited[next_candidate]) {
			return true;
		} else {
			return has_cycle(disqualifications,
					visited, next_candidate);
		}
	}

	return false;
}

bool idisqualif_set::has_cycle(
	const std::vector<std::vector<bool> > &
	disqualifications) const {

	size_t numcands = disqualifications.size();

	for (size_t candidate = 0; candidate < numcands; ++candidate) {
		std::vector<bool> visited(numcands, false);

		if (has_cycle(disqualifications, visited, candidate)) {
			return true;
		}
	}

	return false;
}

void idisqualif_set::print(const std::vector<std::vector<bool> > &
	disqualifications) const {

	size_t numcands = disqualifications.size();

	for (size_t i = 0; i < numcands; ++i) {
		for (size_t j = 0; j < numcands; ++j) {
			if (disqualifications[i][j]) {
				std::cout << (char)('A' + i) << " disqualifies "
					<< (char)('A'+j) << "\n";
			}
		}
	}
}

std::pair<ordering, bool> idisqualif_set::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache, bool winner_only) const {

	//std::cout << "Let's calculate!" << std::endl;

	std::vector<std::vector<bool> > disq_matrix = explore_all_paths(
			papers, hopefuls);

	if (has_cycle(disq_matrix)) {
		print(disq_matrix);
		throw std::runtime_error("idisqualif_set: Yowza! Cycle detected!");
	}

	size_t numcands = hopefuls.size(); // handle sign-compare warning

	// Antisymmetry check
	for (size_t cand = 0; cand < numcands; ++cand) {
		for (size_t i = 0; i < numcands; ++i) {
			if (cand != i && disq_matrix[cand][i] && disq_matrix[i][cand]) {
				std::cout << "[Antisymmetry] failure from " << cand << " to " << i << "\n";
				ballot_tools().print_ranked_ballots(papers);
				std::cout << "[Antisymmetry] Disqualification matrix\n";
				print(disq_matrix);
				std::cout << "[Antisymmetry] Subelection first preference counts\n";
				subelections se;
				se.count_subelections(papers, hopefuls, false);
				se.print_subelection_counts();
				throw std::runtime_error("idisqualif_set: Antisymmetry violation detected!");
			}
		}
	}

	// Turn the set vector into an ordering and return.
	ordering inner_set;

	/*std::vector<double> disqscore = get_proximity_scores(
		papers, hopefuls);*/

	// Consistency check

	for (size_t cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		size_t defeats = 0;
		size_t num_other_hopefuls = 0;

		for (size_t i = 0; i < numcands; ++i) {
			if (!hopefuls[i] || i == cand) {
				continue;
			}
			++num_other_hopefuls;

			if (disq_matrix[cand][i]) {
				++defeats;
			}
		}

		/*if (disqscore[cand] > 0 && defeats < num_other_hopefuls) {
			std::cout << "Oops! Candidate " << cand << " with " << defeats << " defeats vs " << num_other_hopefuls << "\n";
			print(disq_matrix);
			std::cout << "Scores: ";
			std::copy(disqscore.begin(), disqscore.end(), std::ostream_iterator<double>(std::cout, " "));
			std::cout << "\n";
			throw std::runtime_error("Disqualification score > 0 but "
				"didn't defeat everybody");
		}

		if (disqscore[cand] <= 0 && defeats == num_other_hopefuls) {
						std::cout << "Oops! Candidate " << cand << " with " << defeats << " defeats vs " << num_other_hopefuls << "\n";
			print(disq_matrix);
			std::cout << "Scores: ";
			std::copy(disqscore.begin(), disqscore.end(), std::ostream_iterator<double>(std::cout, " "));
			std::cout << "\n";
			throw std::runtime_error("Disqualification score <= 0 but "
				"did defeat everybody");
		}*/

	}

	for (size_t cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		size_t defeated_by = 0;
		size_t defeats = 0;
		size_t num_other_hopefuls = 0;

		for (size_t i = 0; i < numcands; ++i) {
			if (!hopefuls[i] || i == cand) {
				continue;
			}

			++num_other_hopefuls;

			if (disq_matrix[i][cand]) {
				++defeated_by;
			}
			if (disq_matrix[cand][i]) {
				++defeats;
			}
		}

		// defeats also works, but is slightly more manipulable, by
		// about 5%. I'll need to ponder more theory.
		inner_set.insert(candscore(cand, numcands-defeated_by));

		//inner_set.insert(candscore(cand, disqscore[cand]));
	}

	/*std::cout << "DEBUG: Proximity scores: \n";

	for (size_t cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) { continue; }
		std::cout << (char)(cand + 'A') << ": " << disqscore[cand] << "\t";
	}
	std::cout << std::endl;*/

	return std::pair<ordering, bool>(inner_set, false);
}
