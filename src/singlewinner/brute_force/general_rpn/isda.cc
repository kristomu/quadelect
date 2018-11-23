
///////////////////////////////////////////////////////////////

// Smith set and ISDA-related stuff.
// We want this for incrementally building our method if we want
// something that passes ISDA. We first find 3-candidate scenarios with
// full Smith sets, set up appropriate functions for them (in practice
// there's only one, ABCA); then we find 4-candidate scenarios with full
// Smith sets and do the same, and so on. If we have ISDA, 4-candidate
// scenarios with 3-candidate Smith sets don't matter, as they just reduce
// to 3-candidate elections.

#include "gen_custom_function.h"
#include "composition/scenario.h"
#include "composition/equivalences.h"

#include <set>
#include <vector>

std::vector<bool> smith_set(const std::vector<std::vector<bool> > &
	copeland_matrix) {

	std::vector<std::vector<bool> > haspath = copeland_matrix;

	int i, j, k, N = haspath.size();

	for (k = 0; k < N; ++k) {
		for (i = 0; i < N; ++i) {
			if (k != i) {
				for (j = 0; j < N; ++j) {
					if (k != j && i != j) {
						if (haspath[i][k] && haspath[k][j])
							haspath[i][j] = true;
					}
				}
			}
		}
	}

	std::vector<bool> in_smith(N, true);

	for (i = 0; i < N; ++i) {
		for (j = 0; j < N; ++j) {
			if (haspath[j][i] && !haspath[i][j]) {
				in_smith[i] = false;
			}
		}
	}
	return(in_smith);
}

size_t smith_set_size(const std::vector<bool> & smith) {

	size_t count = 0;

	for (bool x: smith) {
		if (x) {
			++count;
		}
	}

	return count;
}

size_t smith_set_size(const copeland_scenario & scenario) {
	return smith_set_size(smith_set(scenario.get_copeland_matrix()));
}

std::set<copeland_scenario> get_nonderived_scenarios(
	size_t desired_smith_set_size,
	const fixed_cand_equivalences & equivalences) {

	std::set<copeland_scenario> out;

	for (const auto & kv: equivalences.get_noncanonical_scenarios()) {
		if (!kv.second.canonical) continue;

		if (smith_set_size(kv.first) == desired_smith_set_size) {
			out.insert(kv.first);
		}
	}

	return out;
}

struct reduction {
	std::vector<int> cand_relabeling;
	copeland_scenario to_scenario;
};

// TODO? function for automatically traipsing through derived
// scenarios after the reduction to find the proper nonderived
// scenario and permutation?

reduction get_ISDA_reduction(const copeland_scenario & in) {

	reduction out;

	// 1. Get the Smith set size of in. If it's equal to numcands, just
	// return in unperturbed.

	std::vector<std::vector<bool> > copeland_matrix =
		in.get_copeland_matrix();

	std::vector<bool> smith_cands = smith_set(copeland_matrix);

	if (smith_set_size(smith_cands) == in.get_numcands()) {
		out.to_scenario = in;
		out.cand_relabeling.resize(in.get_numcands());
		std::iota(out.cand_relabeling.begin(), out.cand_relabeling.end(), 0);

		return out;
	}

	// 2. Get the relabeling (not quite a permutation).
	size_t i, j, out_cand_count = 0;
	for (i = 0; i < copeland_matrix.size(); ++i)	{
		if (!smith_cands[i]) continue;
		out.cand_relabeling.push_back(out_cand_count++);
	}

	// 2. Create a new Copeland matrix of size equal to the Smith set,
	// and copy over from the Copeland matrix, ignoring rows and columns
	// corresponding to the Smith losers.

	std::vector<std::vector<bool> > new_copeland_matrix;

	for (i = 0; i < copeland_matrix.size(); ++i) {
		if (!smith_cands[i]) continue;

		std::vector<bool> new_row;

		for (j = 0; j < copeland_matrix.size(); ++j) {
			if (!smith_cands[j]) continue;
			new_row.push_back(copeland_matrix[i][j]);
		}
		new_copeland_matrix.push_back(new_row);
	}

	out.to_scenario = copeland_scenario(copeland_matrix);

	return out;
}

// We have an eliminate function in ordering_tools. We need a function
// to determine the Smith set for a scenario (not just the number of
// candidates, but who is in it and who's outside). Furthermore, we
// need a function that applies ISDA to a scenario, i.e. returns the
// scenario we would have got if we remove all non-Smith candidates
// while keeping the relative order of those in Smith.

// We also need to more carefully think of how to implement the constraint
// that, when we're to do ISDA for get_test_instance below, A can't be a
// Smith loser. How does this affect what is otherwise an equivalence
// relation? get_permitted_scenarios above doesn't have a limitation of
// that type. I suspect it won't be a problem: all it means is that
// numcands-scenarios with A being a Smith loser won't be anywhere in
// the set, and so get_test_instance will pass them by.

// What we need to find out is if "no reductions that eliminate A" is
// a sensible general rule, and it's just hidden in the
// permitted_scenarios case because A is never eliminated. It doesn't
// look like it, because when we use permitted_scenarios, we want to
// create a ballot of the target type by creating a ballot of the source
// type and then changing it. If that removes A, no biggie, because we're
// only interested in the target anyway. But for ISDA reductions, we want
// to keep the candidate we've been raising or adding top etc. OTOH, if
// it's at all possible to do a reduction that eliminates A, then the
// method must score A as -infinity to begin with!

// So maybe we should keep A-eliminating scenarios and do the reduction
// in test_many or somesuch... Hm. I'm going to take the lazy way out
// and make it an option.

/// ---- //

// Every candidate scenario must be of type numcands and must have
// a Smith set size of smith_set_size.

// This returns the scenarios that reduce, by ISDA, to scenarios of
// fewer candidates, but more than one candidate, and optionally only
// those where A is not eliminated.

// The reason we don't want one-candidate reductions is that these have
// the single winner be the winner by Condorcet, and so are uninteresting
// to test.

std::set<copeland_scenario> get_permissible_ISDA_reductions(
	size_t numcands, bool include_eliminating_A) {

	std::set<copeland_scenario> out;

	// For every candidate scenario of numcands candidates,
	// - get its reduction
	// - if it has the right number of candidates,
	//		(and doesn't eliminate A if that's unwanted),
	//		- add it to the source scenario

	copeland_scenario cur(numcands);
	copeland_scenario base_scenario = cur;

	do {
		reduction red = get_ISDA_reduction(cur);

		if (red.to_scenario.get_numcands() == 1 ||
			red.to_scenario.get_numcands() == numcands) {
			continue;
		}
		// Check whether it eliminates A
		if (!include_eliminating_A) {
			bool found_A = false;
			for (int from_cand : red.cand_relabeling) {
				found_A |= (from_cand == 0);
			}
			if (!found_A) continue;
		}
		out.insert(cur);
	} while (++cur != base_scenario);

	return out;

}

// General setup idea
// get_test_instance gets a request with A, B, as 4 cddt, and A' as 3 cddt.
// The function should first determine what corresponding 4 cddt scenario to
// require, then go through the usual get_test_instance logic to get the
// desired 4-candidate scenarios.
// In the 4-candidate scenario for A', candidate 0 must be in the Smith set,
// and in the 4-candidate scenario for B', the candidate marked as used in B
// must be in the Smith set. (But compositor.cc should handle that.)

// Since compositor already handles so much, I think the best way of doing
// ISDA would be to have a wrapper around get_test_instance, and this
// wrapper function gets called whenever not all of the desired scenarios
// have the same numcands.

// It replaces the lower candidate scenarios with the higher candidate
// scenarios that reduce to them under ISDA, then calls get_test_instance
// as usual, and then reduces back to the lower scenarios under ISDA. This
// entails elimination and permutation. We already have one function for
// this: the one that permutes candidates, but we also need a compose
// permutations function to handle e.g. multiple cand-4 Smith-set 3 scenarios
// all reducing to ABCA as the canonical cyle.

// Reversal should happen outside of the wrapper, so we have...

// reversal -> check for different candidates, etc -> ISDA if reqd ->
// core

// Do we even need permutation composition?

// We should also simplify compositor: for A, when we're randomly generating
// a candidate, we're interested in anything that can be turned into the A
// scenario by permuting candidates (since it doesn't matter to us who A is;
// at least not for the generators we're considering). For B, we want a
// specific scenario. For A', we must have the same ordering as in A,
// since the monotonicity mutation only allows us to raise/add A, not to
// relabel anything. B' is derived from A' with the same candidate focus
// requirement as A->B, i.e if B is candidate 2 before, it must be candidate
// 2 after as well.
// So really, the only latitude we have is in A; but it would be nice to
// automate this. First step should be thus to do some refactoring and
// encapsulation of the data structures used by compositor.
// scenario_reductions, cand_remaps, generalization of nonderived_full
// for all number of candidates < maxnumcands (4 in our case).