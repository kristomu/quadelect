#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "res_minmax.h"

// Everything is as in the indirect disqualification set, except
// we keep track of, for each constraint in a given path, how great
// the margin is. The minimum margin over all constraints for a
// path is the greatest margin for that path; the greatest margin
// over all paths from A to B is the score of A over B. Note that
// if p < 0, this need not be acyclical.

// The chain must start with the root.

void res_minmax::explore_paths(
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

std::vector<std::vector<bool> > res_minmax::explore_all_paths(
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

// This is very slow and very ugly. Fix later. The intent is to keep track
// of the leximax and leximin arrays for disqualification constraint margins,
// to break ties in a way that respects resistant set compliance.

bool is_leximax(std::vector<double> a, std::vector<double> b) {
	std::sort(a.begin(), a.end(), std::greater<double>());
	std::sort(b.begin(), b.end(), std::greater<double>());

	for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
		if (a[i] != b[i]) {
			return a[i] > b[i];
		}
	}

	// If there's still a tie, the longer array wins.
	return a.size() > b.size();
}

bool is_leximin(std::vector<double> a, std::vector<double> b) {
	std::sort(a.begin(), a.end());
	std::sort(b.begin(), b.end());

	for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
		if (a[i] != b[i]) {
			return a[i] < b[i];
		}
	}

	// Not sure about this, but keep it for now.
	return a.size() < b.size();
}

std::vector<double> leximax(std::vector<double> & a,
	std::vector<double> & b) {

	if (is_leximax(a, b)) {
		return a;
	}
	return b;
}

std::vector<double> leximin(std::vector<double> & a,
	std::vector<double> & b) {

	if (is_leximin(a, b)) {
		return a;
	}

	return b;
}

// Quick and dirty
// Let P be a path P_1->..->P_n.
// f(P) = min over all subelections S containing some suffix set Q
//			and a valid leaf R of some prefix of P:
//				(sum of first preferences of Q members) - |Q|/|S|
// Let A and B be two candidates
// strength[A, B] = max over paths P: A->...->B: f(P)

void res_minmax::set_path_strengths(
	const std::vector<bool> & hopefuls,
	const subelections & se,
	std::vector<size_t> & chain_members,
	std::vector<bool> & chain_members_bool,
	size_t leaf, double disq_strength,
	std::vector<double> & root_strengths) const {

	if (chain_members.empty()) {
		throw std::invalid_argument(
			"explore_paths: Chain must contain root!");
	}

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

		// Update the disqualification strength.
		disq_strength = std::min(disq_strength, combined_first_prefs -
				se.num_remaining_voters[se_idx] *
				seen_candidates/(double)se.num_remaining_candidates[se_idx]);
	}

	// Last does disqualify leaf!

	// Set disqualification and add the leaf to the chain.
	root_strengths[leaf] = std::max(root_strengths[leaf], disq_strength);
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

		set_path_strengths(hopefuls, se, chain_members,
			chain_members_bool, candidate_leaf,
			disq_strength, root_strengths);
	}

	// Remove the leaf member from the chain so that whoever is
	// calling us can try another branch.
	chain_members.pop_back();
	assert(chain_members_bool[leaf]);
	chain_members_bool[leaf] = false;
}

std::vector<std::vector<double> > res_minmax::set_all_path_strengths(
	const election_t & papers,
	const std::vector<bool> & hopefuls) const {

	size_t numcands = hopefuls.size();

	// Setting true here makes it a lot more monotone, but not
	// fully, and it also causes antisymmetry problems (I think).
	// But Micawber very much holds.
	subelections se;
	se.count_subelections(papers, hopefuls, tiebreak);

	std::vector<std::vector<double> > strength_matrix(
		numcands, std::vector<double>(numcands,
			-std::numeric_limits<double>::infinity()));

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

			set_path_strengths(hopefuls, se, chain_members,
				chain_members_bool, leaf,
				std::numeric_limits<double>::infinity(),
				strength_matrix[root]);
		}
	}

	return strength_matrix;
}

// Standard DFS search to detect a cycle.

bool res_minmax::has_cycle(
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

bool res_minmax::has_cycle(
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

void res_minmax::print(const std::vector<std::vector<bool> > &
	disqualifications) const {

	size_t numcands = disqualifications.size();

	for (size_t i = 0; i < numcands; ++i) {
		for (size_t j = 0; j < numcands; ++j) {
			if (disqualifications[i][j]) {
				std::cout << (char)('A' + i) << " indirectly disqualifies "
					<< (char)('A'+j) << "\n";
			}
		}
	}
}

std::pair<ordering, bool> res_minmax::elect_inner(
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
	size_t cand, i;

	// Antisymmetry check
	for (cand = 0; cand < numcands; ++cand) {
		for (i = 0; i < numcands; ++i) {
			if (cand != i && disq_matrix[cand][i] && disq_matrix[i][cand]) {
				std::cout << "[Antisymmetry] failure from " << cand << " to " << i << "\n";
				ballot_tools().print_ranked_ballots(papers);
				std::cout << "[Antisymmetry] Disqualification matrix\n";
				print(disq_matrix);
				std::cout << "[Antisymmetry] Subelection first preference counts\n";
				subelections se;
				se.count_subelections(papers, hopefuls, false);
				se.print_subelection_counts();
				throw std::runtime_error("idisqualif_set: Antisymmetry violation "
					"detected!");
			}
		}
	}

	// Debug
	std::vector<std::vector<double> > strength = set_all_path_strengths(
			papers, hopefuls);

	double numvoters = 0;
	for (auto p: papers) {
		numvoters += p.get_weight();
	}

	condmat strength_cm(numcands, numvoters, CM_PAIRWISE_OPP);
	strength_cm.zeroize(numcands);

	/*for (cand = 0; cand < numcands; ++cand) {
		for (i = 0; i < numcands; ++i) {
			strength[i][cand] = std::max(0.0, strength[i][cand]);
		}
	}*/

	/*for (cand = 0; cand < numcands; ++cand) {
		std::copy(strength[cand].begin(), strength[cand].end(),
			std::ostream_iterator<double>(std::cout, " "));
		std::cout << "\n";
	}*/

	// Augment by the minimum measure.
	for (cand = 0; cand < numcands; ++cand) {
		for (i = 0; i < numcands; ++i) {
			if (cand == i) {
				continue;
			}

			// Hack.

			bool better_by_lex = is_leximax(strength[cand], strength[i]);
			bool better_by_str = (strength[cand] > strength[i]);

			bool better_in_all = false;

			switch (type) {
				case RM_LEXIMAX:
					better_in_all = better_by_lex;
					break;
				case RM_STRENGTH:
					better_in_all = better_by_str;
					break;
				case RM_LSTRENGTH:
					better_in_all = better_by_lex;
					if (!better_in_all &&
						!is_leximax(strength[i], strength[cand])) {
						better_in_all = better_by_str;
					}
					break;
				case RM_STRLEX:
					better_in_all = better_by_str;
					if (strength[cand][i] == strength[i][cand]) {
						better_in_all = better_by_lex;
					}
					break;
				case RM_CONDORCET:
					better_in_all = false; // not used
					break;
			}

			disq_matrix[cand][i] = better_in_all;

			// uh oh
			strength_cm.add(cand, i, std::max(strength[cand][i], 0.0));
		}
	}

	// For some reason, this doesn't work! Ext-minmax produces markedly
	// inferior results to plain old is_leximax, even though it should
	// be exactly the same. I might need to find out why, later.

	if (type == RM_CONDORCET) {
		return base_method->pair_elect(strength_cm, hopefuls, false);
	}

	/*if (has_cycle(disq_matrix)) {
		print(disq_matrix);
		throw std::runtime_error("idisqualif_set: Yowza! Cycle detected!");
	}*/


	// Turn the set vector into an ordering and return.
	ordering inner_set;

	// Consistency check

	for (cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		size_t defeats = 0;
		size_t num_other_hopefuls = 0;

		for (i = 0; i < numcands; ++i) {
			if (!hopefuls[i] || i == cand) {
				continue;
			}
			++num_other_hopefuls;

			if (disq_matrix[cand][i]) {
				++defeats;
			}
		}

	}

	for (cand = 0; cand < numcands; ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}
		size_t defeated_by = 0;

		for (i = 0; i < numcands; ++i) {
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
}
