#include <cassert>
#include <iostream>
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
//		If the whole chain_members list, *and* leaf is in S,
//			If the sum of first prefs of the chain_members list, does
//				not exceed k/|S|, then return (last doesn't
//				disqualify leaf through the root), where k is the
//				length of that list.
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

	size_t root = chain_members[0],
		   last = *chain_members.rbegin();
	size_t num_subelections = se.hopeful_power_set.size();

	// TODO: Check that this also verifies pairwise.

	for (size_t se_idx = 0; se_idx < num_subelections; ++se_idx) {
		// If leaf isn't in it, then it's not interesting.
		if (!se.hopeful_power_set[se_idx][leaf]) {
			continue;
		}

		/*if (se.num_remaining_candidates[se_idx] == 2) {
			std::cout << "debug, pairwise included" << std::endl;
		}*/

		// If last and leaf are in it but root isn't, check
		// ordinary disqualification.

		if (se.hopeful_power_set[se_idx][last] &&
			!se.hopeful_power_set[se_idx][root]) {
			// fp(last)_S <= numvoters/|S|, or for numerical stability,
			// |S| * fp(last)_S <= numvoters
			if (se.num_remaining_candidates[se_idx] *
				se.first_pref_scores[se_idx][last] <=
				se.num_remaining_voters[se_idx]) {
				// last doesn't disqualify leaf.
				return;
			}
		}

		// Skip if not everybody in the current chain is in the
		// subelection.
		bool all_members = true;
		double combined_first_prefs = 0;
		for (size_t chain_cand : chain_members) {
			all_members &= se.hopeful_power_set[se_idx][chain_cand];
			combined_first_prefs += se.first_pref_scores[se_idx][chain_cand];

			if (!all_members) {
				continue;
			}
		}
		if (!all_members) {
			continue;
		}

		// If we fail to disqualify leaf through the root, abort.
		// combined_first_prefs <= numvoters * chain_members.size() / |S|
		// i.e. |S| * cfs <= numvoters * chain_members.size()
		if (se.num_remaining_candidates[se_idx] *
			combined_first_prefs <=
			se.num_remaining_voters[se_idx] * chain_members.size()) {
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
	se.count_subelections(papers, hopefuls, true);

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

	std::vector<std::vector<bool> > disq_matrix = explore_all_paths(
			papers, hopefuls);

	if (has_cycle(disq_matrix)) {
		print(disq_matrix);
		throw std::runtime_error("idisqualif_set: Yowza! Cycle detected!");
	}

	size_t numcands = hopefuls.size(); // handle sign-compare warning

	// Turn the set vector into an ordering and return.
	ordering inner_set;

	for (size_t cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		size_t defeated_by = 0;

		for (size_t i = 0; i < numcands; ++i) {
			if (!hopefuls[i] || i == cand) {
				continue;
			}
			if (disq_matrix[i][cand]) {
				++defeated_by;
			}
		}

		inner_set.insert(candscore(cand, numcands-defeated_by));
	}

	return std::pair<ordering, bool>(inner_set, false);

	/*
	// TODO, don't make it count, just do a blank matrix.
	condmat matrix(papers, num_candidates, CM_PAIRWISE_OPP);

	for (size_t incumbent = 0; incumbent < numcands; ++incumbent) {
		for (size_t challenger = 0; challenger < numcands; ++challenger) {
			matrix.set(incumbent, challenger, 0);
			if (!hopefuls[incumbent])  { continue; }
			if (!hopefuls[challenger] || incumbent == challenger) { continue; }

			if (disq_matrix[incumbent][challenger]) {
				matrix.set(incumbent, challenger, 1);
				if (disq_matrix[challenger][incumbent]) {
					throw std::runtime_error("Yowza! Cycle detected!");
				}
			}
		}
	}

	return smith_set().pair_elect(matrix, hopefuls,
		cache, winner_only);
	*/
}