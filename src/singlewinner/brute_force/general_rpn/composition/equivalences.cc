// The functions below handle the following:
//		- Determining the equivalence classes for a k-candidate election
//		- Taking an election (in vector format) and relabeling it either
//			to be a particular scenario or have a particular candidate
//			as candidate 0, and returning what scenario and candidate
//			permutation this corresponds to.

// Also some actual composition testing.

// TODO: Rewrite these comments. Also clean up the headers.
// The comments are even more in need of a rewrite now...

#include "../../../../tools/tools.h"
#include "../../../../tools/factoradic.h"
#include "../../../../ballots.h"

#include "../../../../generator/impartial.h"
#include "../../../../pairwise/matrix.h"
#include "../gen_custom_function.h"

#include <numeric>
#include <algorithm>
#include <stdexcept>

#include <iostream>
#include <iterator>

#include "equivalences.h"

// We want to construct an election method by composing different
// gen_custom_functions, one for each Copeland scenario. We have to take
// two things into account:
// First, a gen_custom_function only provides the score of the first
// candidate (A) as a function of the ballot data. So to find the score of
// other candidates, we need to relabel the candidates so that the candidate
// we're interested in becomes the first candidate.
// Second, some Copeland scenario,candidate pairs are equivalent to each
// other. We thus want to find the minimal subset of scenarios so that every
// other scenario,candidate pair can be reduced to one of these scenarios
// with the candidate in question now being candidate 0.

// Two Copeland scenarios are equivalent if you can use a gen_custom_function
// assigned to one to determine A's score from the perspective of the other.
// In other words, if there's a permutation of candidates that keeps the first
// candidate the first, but turns scenario X into scenario Y, then X and Y
// are equivalent. This is symmetric and transitive.

// We thus first need to determine the minimum set of Copeland scenarios so
// that every scenario is equivalent to one of these.

// To do so, we do the following:
// [TODO: Write a proper strategy here]
// Possibly use the term "canonical" instead of "nonderived".

// Because there should be only one permutation per candidate, we can
// compactly store both the derived to non-derived mapping and the
// candidate rotation mapping in a single map. If the scenario is derived,
// then its mapping for candidate 0 (i.e. what to use if we want the score
// for the first candidate) can point at the non-derived scenario it is
// derived from.

// ASSOCIATED PROOF that if we have all nonderived for candidate 0, every
// strategy will have a permutation that maps candidate X to 0 and goes to
// a nonderived scenario:
//		By pigeonhole, any given permutation that turns candidate X to 0
//		will turn the short form into some scenario's short form.
//		That scenario gives the original scenario's score for X by
//		giving the result for 0.
//		Either that scenario is nonderived, in which case we're done; or
//		it has a mapping that preserves candidate 0 as candidate 0 and
//		goes to a nonderived scenario. In the latter case, the composition
//		of the first permutation and that permutation gets us from candidate
//		X in the first scenario to candidate 0 in some nonderived scenario,
//		QED.

// --- //

// Gives every permutation where candidate cand is relabeled to the first
// candidate.
std::vector<std::vector<int> >
fixed_cand_equivalences::all_permutations_centered_on(int cand,
	size_t numcands) const {

	// Create the first such permutation
	std::vector<int> perm(numcands);
	std::iota(perm.begin(), perm.end(), 0);
	std::swap(perm[cand], perm[0]);
	sort(perm.begin()+1, perm.end());

	std::vector<std::vector<int> > perms;

	// and go through every subsequent permutation of the remaining
	// candidates.
	do {
		perms.push_back(perm);
	} while (std::next_permutation(perm.begin()+1, perm.end()));

	return perms;
}

std::map<copeland_scenario, isomorphism>
fixed_cand_equivalences::get_noncanonical_scenario_reductions(
	size_t numcands, bool verbose) const {

	// Note that we go through the scenarios in reverse order. The
	// reason for doing so is historical: I considered the 3-cycle ABCA
	// to be a nonderived one (and ACBA to be derived from it), but
	// going in ascending order makes ACBA the nonderived cycle.

	std::map<copeland_scenario, isomorphism> reductions;

	copeland_scenario base_scenario(numcands);
	--base_scenario;
	copeland_scenario cur = base_scenario;

	isomorphism cur_reduction;

	std::vector<int> identity_permutation(numcands);
	std::iota(identity_permutation.begin(), identity_permutation.end(), 0);

	// For every possible scenario
	do {
		bool canonical = true, has_populated_isomorphism = false;

		cur_reduction.cand_permutations.clear();

		// For every way of permuting the candidates
		for (const std::vector<int> & permutation:
			all_permutations_centered_on(0, numcands)) {

			// Create permuted scenario
			copeland_scenario permuted = cur;
			permuted.permute_candidates(permutation);

			// Check if we've seen a nonderived that matches it. If not,
			// skip.
			if (reductions.find(permuted) == reductions.end() ||
				!reductions.find(permuted)->second.canonical) {
				continue;
			}

			// We've already seen it, so..

			// Ensure my transitivity hunch is correct. (Also works as a
			// bug check.)
			assert(!has_populated_isomorphism ||
				reductions.find(permuted)->second.to_scenario ==
				cur_reduction.to_scenario);

			// Add the current permutation to the list of such.
			canonical = false;
			has_populated_isomorphism = true;
			cur_reduction.to_scenario = permuted;
			cur_reduction.cand_permutations.push_back(permutation);
			cur_reduction.canonical = false;

			// This should be dumped into its own function

			if (verbose) {

				std::cout << cur.to_string() << " is derived" << "\t";
				std::copy(permutation.begin(), permutation.end(),
					std::ostream_iterator<int>(std::cout, " "));
				std::cout << "\t" << permuted.to_string() << std::endl;
			}
		}

		/*std::cout << " Out of permutation loop " << std::endl;*/

		// If it's canonical, mark it as such by making it isomorphic
		// only to itself.
		if (canonical) {
			cur_reduction.to_scenario = cur;
			cur_reduction.cand_permutations.push_back(identity_permutation);
			cur_reduction.canonical = true;
			reductions[cur] = cur_reduction;
			if (verbose) {
				std::cout << cur.to_string() << " is nonderived" <<
					std::endl;
			}
		} else {
			// Otherwise, add the reduction to the list.
			reductions[cur] = cur_reduction;
		}

	} while (--cur != base_scenario);

	return reductions;

}

// Determine how to use nonderived scenarios to get the score for any
// scenario and any candidate. The vector works like this:
// candidate_remapping[cand][start_scenario] gives the isomorphism
// that in turn gives the copeland_scenario required to get cand's score,
// as well as how to permute the candidates, starting at start_scenario,
// to get to that copeland_scenario.

// TODO: Refactor this as the inner loop is very much like the inner loop
// in the function above.

std::map<copeland_scenario, isomorphism>
fixed_cand_equivalences::get_one_candidate_remapping(
	size_t numcands, int current_candidate,
	const std::map<copeland_scenario, isomorphism> &
	noncanonical_reductions) const {

	std::map<copeland_scenario, isomorphism> cand_remapping;

	copeland_scenario cur(numcands);
	copeland_scenario base_scenario = cur;

	isomorphism cur_reduction;

	// We can increment here because the order we go through scenarios
	// doesn't matter.

	do {
		cur_reduction.cand_permutations.clear();

		bool has_populated_isomorphism = false;

		for (const std::vector<int> & permutation:
			all_permutations_centered_on(current_candidate, numcands)) {

			// Create permuted scenario
			copeland_scenario permuted = cur;
			permuted.permute_candidates(permutation);

			// Check if we've seen a canonical that matches it. If not,
			// skip.
			if (noncanonical_reductions.find(permuted) ==
				noncanonical_reductions.end()
				|| !noncanonical_reductions.find(permuted)->second.canonical) {
				continue;
			}

			assert(!has_populated_isomorphism ||
				noncanonical_reductions.find(permuted)->second.to_scenario ==
				cur_reduction.to_scenario);

			// Add the current permutation to the list of such.
			has_populated_isomorphism = true;
			cur_reduction.to_scenario = permuted;
			cur_reduction.cand_permutations.push_back(permutation);
			cur_reduction.canonical = false; // Not relevant

			/*std::cout << cur.to_string() << " with cand " << current_candidate
				<< " maps to " << "\t";
			std::copy(permutation.begin(), permutation.end(),
				std::ostream_iterator<int>(std::cout, " "));
			std::cout << "\t" << permuted.to_string() << std::endl;*/
		}

		assert(has_populated_isomorphism);

		cand_remapping[cur] = cur_reduction;
	} while (++cur != base_scenario);

	return cand_remapping;
}

std::vector<std::map<copeland_scenario, isomorphism> >
fixed_cand_equivalences::get_all_candidate_remappings(size_t numcands,
	const std::map<copeland_scenario, isomorphism> &
	noncanonical_reductions) const {

	std::vector<std::map<copeland_scenario, isomorphism> > remappings;
	// The candidate remapping for candidate 0 onto canonical scenarios
	// is just the noncanonical reduction.
	remappings.push_back(noncanonical_reductions);

	for (size_t i = 1; i < numcands; ++i) {
		remappings.push_back(get_one_candidate_remapping(numcands, i,
				noncanonical_reductions));
	}

	return remappings;
}

std::map<copeland_scenario, std::set<copeland_scenario> >
fixed_cand_equivalences::get_scenario_equivalences(const
	std::vector<std::map<copeland_scenario, isomorphism> > &
	candidate_remappings_in) const {

	// Just go through candidate_remappings_in for all candidates.
	// equivalences[scenario] = union over all candidates c:
	// candidate_remappings_in[c][scenario].to_scenario.

	std::map<copeland_scenario, std::set<copeland_scenario> > out;

	for (const auto & remappings_for_one_cand : candidate_remappings_in) {
		for (const auto & key_val : remappings_for_one_cand) {
			out[key_val.first].insert(key_val.second.to_scenario);
		}
	}

	return out;
}

// Get all fixed cand equivalences up to (and including) this number of
// candidates.

std::map<int, fixed_cand_equivalences> get_cand_equivalences(
	int max_numcands) {

	std::map<int, fixed_cand_equivalences> other_equivs;

	for (int i = 3; i <= max_numcands; ++i) {
		other_equivs.insert(std::pair<int, fixed_cand_equivalences>(i,
				fixed_cand_equivalences(i)));
	}

	return other_equivs;
}