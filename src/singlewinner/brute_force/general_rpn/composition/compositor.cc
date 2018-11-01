// This class handles the composition of different gen_custom_functions into
// an election method for k candidates. We do the composition by relying on a
// concept called a Copeland scenario. See the file for copeland_scenario
// for information about what a Copeland scenario is.

// For speed, we use a k!-long std::vector<double> to represent a concrete
// election. The vector gives the number of voters who voted each of the
// k! possible rankings in a k-candidate election.

// The class handles the following functions (might be separated later):
//		- Determining the equivalence classes for a k-candidate election
//		- Taking an election (in vector format) and relabeling it either
//			to be a particular scenario or have a particular candidate
//			as candidate 0, and returning what scenario and candidate
//			permutation this corresponds to.

// TODO: Rewrite these comments.

#include "../../../../tools/tools.h"
#include "../../../../tools/factoradic.h"
#include "../../../../ballots.h"

#include "../../../../generator/impartial.h"
#include "../../../../pairwise/matrix.h"
#include "../../../../tests/tests/monotonicity/mono_add.h"
#include "../../../../tests/tests/monotonicity/mono_raise.h"
#include "../rpn_evaluator.cc"

#include <set>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <stdexcept>

#include <iostream>
#include <iterator>

#include "scenario.cc"

// We want to construct an election method by composing different 
// gen_custom_functions, one for each Copeland scenario. We have to take
// two things into account: 
// First, a gen_custom_function only provides the score of the first candidate
// (A) as a function of the ballot data. So to find the score of other 
// candidates, we need to relabel the candidates so that the candidate we're 
// interested in becomes the first candidate.
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
//	- For every possible scenario,candidate pair:
//		- Go through every way of permuting the candidates, starting from
//		  that scenario, that turns the specified candidate into the first
//		  candidate.
//			If the scenario after rotation matches any that we've seen
//			before, then 
//			If any fit, then this scenario can be reduced to a nonderived 
//			one we've already seen before: record that it is equivalent to
//			the other scenario with the given permutation of candidates. 
//		- If there is no such reduction, mark the current scenario as
//			nonderived so future scenarios can be checked against it.

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
std::vector<std::vector<int> > all_permutations_centered_on(int cand, 
	int numcands) {

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

// After permuting according to cand_permutation, we end up in
// to_scenario.

struct isomorphism {
	std::vector<std::vector<int> > cand_permutations;
	copeland_scenario to_scenario;
	bool derived;
};

std::map<copeland_scenario, isomorphism> get_derived_scenario_reductions(
	int numcands) {

	// Note that we go through the scenarios in reverse order. The
	// reason for doing so is historical: I considered the 3-cycle ABCA
	// to be a nonderived one (and ACBA to be derived from it), but
	// going in ascending order makes ACBA the nonderived cycle.

	std::map<copeland_scenario, isomorphism> reductions;

	copeland_scenario base_scenario(numcands);
	--base_scenario;
	copeland_scenario cur = base_scenario;

	int i = 0;

	isomorphism cur_reduction;

	std::vector<int> identity_permutation(numcands);
	std::iota(identity_permutation.begin(), identity_permutation.end(), 0);

	// For every possible scenario
	do {
		bool derived = false, has_populated_isomorphism = false;

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
				reductions.find(permuted)->second.derived) {
				continue;
			}

			// We've already seen it, so..

			// Ensure my transitivity hunch is correct. (Also works as a
			// bug check.)
			assert (!has_populated_isomorphism || 
				reductions.find(permuted)->second.to_scenario ==
				cur_reduction.to_scenario);

			// Add the current permutation to the list of such.
			derived = true;
			has_populated_isomorphism = true;
			cur_reduction.to_scenario = permuted;
			cur_reduction.cand_permutations.push_back(permutation);
			cur_reduction.derived = true;

			std::cout << cur.to_string() << " is derived" << "\t";
			std::copy(permutation.begin(), permutation.end(),
				std::ostream_iterator<int>(std::cout, " "));
			std::cout << "\t" << permuted.to_string() << std::endl;
		}

		/*std::cout << " Out of permutation loop " << std::endl;*/

		// If it's derived, add the reduction to the list.
		if (derived) {
			reductions[cur] = cur_reduction;
		} else {
			// If it's non-derived, mark it as such by making it isomorphic
			// only to itself.
			cur_reduction.to_scenario = cur;
			cur_reduction.cand_permutations.push_back(identity_permutation);
			cur_reduction.derived = false;
			reductions[cur] = cur_reduction;
			std::cout << cur.to_string() << " is nonderived" << std::endl;
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

std::map<copeland_scenario, isomorphism> get_candidate_remapping(int numcands,
	int current_candidate,
	const std::map<copeland_scenario, isomorphism> & derived_reductions) {

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

			// Check if we've seen a nonderived that matches it. If not,
			// skip.
			if (derived_reductions.find(permuted) == derived_reductions.end() 
				|| derived_reductions.find(permuted)->second.derived) {
				continue;
			}

			assert (!has_populated_isomorphism || 
				derived_reductions.find(permuted)->second.to_scenario ==
				cur_reduction.to_scenario);

			// Add the current permutation to the list of such.
			has_populated_isomorphism = true;
			cur_reduction.to_scenario = permuted;
			cur_reduction.cand_permutations.push_back(permutation);
			cur_reduction.derived = true; // Not relevant

			std::cout << cur.to_string() << " with cand " << current_candidate 
				<< " maps to " << "\t";
			std::copy(permutation.begin(), permutation.end(),
				std::ostream_iterator<int>(std::cout, " "));
			std::cout << "\t" << permuted.to_string() << std::endl;
		}

		assert(has_populated_isomorphism);

		cand_remapping[cur] = cur_reduction;
	} while (++cur != base_scenario);

	return cand_remapping;
}

std::vector<std::map<copeland_scenario, isomorphism> > 
	get_candidate_remappings(int numcands,
	const std::map<copeland_scenario, isomorphism> & derived_reductions) {

	std::vector<std::map<copeland_scenario, isomorphism> > remappings;
	// The candidate remapping for candidate 0 onto nonderived scenarios
	// is just the derived reduction.
	remappings.push_back(derived_reductions);

	for (int i = 1; i < numcands; ++i) {
		remappings.push_back(get_candidate_remapping(numcands, i,
			derived_reductions));
	}

	return remappings;
}

///////////////////////////////////////////////////////////////

// Smith set stuff
// We want this for incrementally building our method if we want
// something that passes ISDA. We first find 3-candidate scenarios with
// full Smith sets, set up appropriate functions for them (in practice
// there's only one, ABCA); then we find 4-candidate scenarios with full
// Smith sets and do the same, and so on. If we have ISDA, 4-candidate
// scenarios with 3-candidate Smith sets don't matter, as they just reduce
// to 3-candidate elections.

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

int smith_set_size(const copeland_scenario & scenario) {
	std::vector<bool> smith = smith_set(scenario.get_copeland_matrix());

	int count = 0;

	for (bool x: smith) {
		if (x) {
			++count;
		}
	}

	return count;
}

std::set<copeland_scenario> get_nonderived_scenarios(
	int desired_smith_set_size,
	const std::map<copeland_scenario, isomorphism> & derived_reductions) {

	std::set<copeland_scenario> out;

	for (const auto & kv: derived_reductions) {
		if (kv.second.derived) continue;

		if (smith_set_size(kv.first) == desired_smith_set_size) {
			out.insert(kv.first);
		}
	}

	return out;
}


// What's next?
// ballot_group --> factorial format vector
// list<ballot_group> --> factorial format vector
// list<ballot_group> --> list<ballot_group> permuting candidate names

// We may need the inverse of kth_permutation for this. 

// Those give us a way of generating a random election and getting the
// vote vectors for all candidates (this will necessarily be a part of
// any future election_method wrapper anyway).

// After this, we can then construct monotonicity pairs and winnow in
// parallel as shown in autocloneproofing.txt

void inc_ballot_vector(const ballot_group & ballot_to_add, 
	std::vector<double> & permutation_count_vector, int numcands) {

	// Get the ordering.

	std::vector<int> linear_ordering;

	// for detecting equal-rank (which we don't support)
	double last_score = INFINITY;

	// TODO: Fix. Error ultimately resides in twotests or monotonicity.
	/*if (!ballot_to_add.complete) {
		throw new std::runtime_error(
			"Trying to add incomplete ballot to ballot vector!");
	}*/

	for (candscore cur_cand_vote : ballot_to_add.contents) {
		if (cur_cand_vote.get_score() == last_score) {
			throw new std::runtime_error(
				"Trying to add ballot with equal-rank to ballot vector!");
		}
		linear_ordering.push_back(cur_cand_vote.get_candidate_num());
		last_score = cur_cand_vote.get_score();
	}

	if (linear_ordering.size() != numcands) {
		throw new std::runtime_error("Trying to add truncated ballot to ballot vector!");
	}

	// Increment the relevant index.
	permutation_count_vector[factoradic().permutation_number(linear_ordering, 
		numcands, 0)] += ballot_to_add.weight;
}

std::vector<double> get_ballot_vector(const list<ballot_group> & election,
	int numcands) {

	std::vector<double> ballot_vector(factorial(numcands));

	for (const ballot_group & ballot : election) {
		inc_ballot_vector(ballot, ballot_vector, numcands);
	}

	return ballot_vector;
}

list<ballot_group> permute_election_candidates(
	const list<ballot_group> & election_in,
	const std::vector<int> & candidate_permutation) {

	// order = {3, 2, 0, 1} means
	// what will be the first candidate (A) in the output is D (#3) in the input

	// We first need to invert the permutation, so that inv_permutation[x]
	// is the new candidate number of the candidate that used to be x.
	// Fortunately, that is pretty easy.

	int numcands = candidate_permutation.size();
	std::vector<int> inv_permutation(numcands, -1);

	for (int i = 0; i < numcands; ++i) {
		inv_permutation[candidate_permutation[i]] = i;
	}

	list<ballot_group> election_out;

	// Now just go through and relabel everything.
	for (const ballot_group & ballot_in: election_in) {
		ballot_group ballot_out = ballot_in;
		ballot_out.contents.clear();

		for (candscore cur_cand_vote: ballot_in.contents) {
			ballot_out.contents.insert(
				candscore(inv_permutation[cur_cand_vote.get_candidate_num()],
					cur_cand_vote.get_score()));
		}

		election_out.push_back(ballot_out);
	}

	return election_out;

}

// Testing testing
void show_transitions(const copeland_scenario & desired_scenario, int numcands,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
	candidate_remappings, rng & randomizer) {

	impartial ic(true);

	mono_add_top mattest(false, false);

	list<ballot_group> base_election;
	condmat condorcet_matrix(CM_PAIRWISE_OPP);

	do {
                // Use an odd number of voters to avoid ties.
                base_election = ic.generate_ballots(2 * randomizer.lrand(5, 50) + 1,
                        numcands, randomizer);
                condorcet_matrix.zeroize();
                condorcet_matrix.count_ballots(base_election, numcands);
        } while (copeland_scenario(&condorcet_matrix) != desired_scenario);

	monotonicity * mono_test = &mattest;

        std::vector<int> mono_data = mono_test->generate_aux_data(base_election,
                numcands);

        mono_data[0] = 0;

        std::pair<bool, list<ballot_group> > alteration = mono_test->
                rearrange_ballots(base_election, numcands, mono_data);

        // didn't succeed in altering the ballot set in favor of A, so leave
        if (!alteration.first) {
                return;
        }

	try {
	        condorcet_matrix.zeroize();
        	condorcet_matrix.count_ballots(alteration.second, numcands);
		std::cout << "Base for A is " << copeland_scenario(&condorcet_matrix).to_string() << std::endl;
	} catch (const std::exception & e) { 
		return;
	}

	std::vector<std::vector<double> > base_election_different_cands,
                modified_election_different_cands;

	std::vector<copeland_scenario> base_c, modified_c;

        for (int i = 0; i < numcands; ++i) {
                // TODO: Multiple cand permutations may exist. Handle it.
                std::vector<int> center_on_this_cand = candidate_remappings[i].
                        find(desired_scenario)->second.cand_permutations[0];

		condorcet_matrix.zeroize();
		condorcet_matrix.count_ballots(permute_election_candidates(
			base_election, center_on_this_cand), numcands);
		char cand_name = i + 'A';
		std::string base = std::string(1, cand_name), modified = base + "'";

		std::cout << base << ": " << copeland_scenario(&condorcet_matrix).to_string() << std::endl;
		base_c.push_back(copeland_scenario(&condorcet_matrix));

		condorcet_matrix.zeroize();
		condorcet_matrix.count_ballots(permute_election_candidates(
			alteration.second, center_on_this_cand), numcands);
		
		std::cout << modified << ": " << copeland_scenario(&condorcet_matrix).to_string() << std::endl;
		modified_c.push_back(copeland_scenario(&condorcet_matrix));
	}

	for (int i = 0; i < numcands; ++i) {
		// Check if we have scenario(k') = scenario(A)
		// and scenario(k) = scenario(A')

		if (base_c[0] == modified_c[i] && base_c[i] == modified_c[0]) {
			std::cout << "Pay attention! i = " << i << std::endl;
		}
	}

         /*       base_election_different_cands.push_back(
                        get_ballot_vector(permute_election_candidates(
                                base_election, center_on_this_cand), numcands));
                modified_election_different_cands.push_back(
                        get_ballot_vector(permute_election_candidates(
                                alteration.second, center_on_this_cand), numcands));
        }*/

	std::cout << std::endl;

}

// Reconstruction of this testing system: We have a number of acceptable
// scenarios. We want every candidate rotation to give scenarios which are
// among these. The first function should generate a bunch of before-
// and-after elections. The before elections must be within the "acceptable
// before" set, and the after elections must be the same scenario as the
// corresponding before scenario. (We might let this be a function of an
// after acceptable set and the before scenario, so we can extend from 
// 3-candidate elections. There the idea would be to let either before or
// after be something with a 3-candidate Smith set, and the other be some
// 4-candidate Smith set -- then we insist upon the 3-candidate result being
// according to one or more known 3-candidate methods, in practice fpA-fpC.)

// Furthermore, there should be equally many before-elections of each
// scenario type (each scenario pair, if we make afters more general).

// Then we have a "test one election on everybody" routine that spits out
// a vector<double> of scores.

// Test_many is given a before scenario (alternatively before and after)
// and the elections list, and then for each election, runs it on everybody
// and collates the results as in test_many below.

// Then, there's a filter: this filter goes through a 2D map of ints,
// removing those that are disqualified according to the
// monotonicity test -- unless an "initial population" boolean is set to
// true, in which case it populates the map with those that pass, instead.

// Finally, there's an enumerator, which is simple enough: it starts with
// the first two permitted scenarios, goes through all still allowed pairs,
// and for each of those, tries to extend onto the third scenario, and
// then onto a fourth, like this:

// 	scenario 1   scenario 2   
//	19           20           
//               scenario 2   scenario 3
//               nothing starting in 20 - no viable method

//  19           30
//               scenario 2   scenario 3
//               30           40
//  scenario 1                scenario 3
//  19                        40           (if this didn't exist, not viable)
//                            scenario 3   scenario 4
//                            40           50
//               scenario 2                scenario 4
//               30                        50           (ditto)

// (19, 30, 40, 50) is a viable method.

// (We can also very quickly check cross-monotonicity in a similar way, 
// once we have the candidates. We only need to generate a bunch of 
// elections where A, B, C, D are all different, then get the results for
// A, B, C, D, A', B', C', D' for every method. This is probably easier to
// do on an election-by-election basis.

// There's a problem here. We don't ascertain that after improving A's
// condition, the scenario is the same. TODO: Think about how to
// incorporate that snag. Later.
std::vector<std::vector<double> > test(
	const copeland_scenario & desired_scenario, int numcands,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
	candidate_remappings, const std::vector<algo_t> & functions_to_test,
	rng & randomizer) {
	// Ad hoc testing scheme. Improve later.

	// Generate a random ballot set.
	impartial ic(true);

	mono_add_top mattest(false, false);
	mono_raise mrtest(false, false);

	list<ballot_group> base_election;
	condmat condorcet_matrix(CM_PAIRWISE_OPP);

	// 1. Generate a base election until we have something that follows
	// the desired base scenario. (We need to do this so that B's perspective
	// is always the same scenario, C's is and so on, so that the vector
	// of results can be compared.)

	do {
		// Use an odd number of voters to avoid ties.
		base_election = ic.generate_ballots(2 * randomizer.lrand(5, 50) + 1, 
			numcands, randomizer);
		condorcet_matrix.zeroize();
		condorcet_matrix.count_ballots(base_election, numcands);
	} while (copeland_scenario(&condorcet_matrix) != desired_scenario);

	// 2. Add an A-first ballot somewhere.

	// VERY hacky. We make use of that data[0] specifies the candidate
	// this is supposed to benefit, which is here always the first
	// candidate (A).

	// Check for *both* mono-raise and mono-add-top. We need to get rid
	// of as many methods as possible.
	monotonicity * mono_test = &mattest;
	if (randomizer.drand() < 0.5) mono_test = &mrtest;

	std::vector<int> mono_data = mono_test->generate_aux_data(base_election,
		numcands);

	mono_data[0] = 0;

	std::pair<bool, list<ballot_group> > alteration = mono_test->
		rearrange_ballots(base_election, numcands, mono_data);

	// didn't succeed in altering the ballot set in favor of A, so leave
	if (!alteration.first) {
		return std::vector<std::vector<double> >();
	}

	condorcet_matrix.zeroize();
	condorcet_matrix.count_ballots(alteration.second, numcands);

	// Hack for when the alteration gets us into another scenario. To do
	// later: handle this properly, as it will strengthen the constraint.
	try {
		if (copeland_scenario(&condorcet_matrix) != desired_scenario) {
			return std::vector<std::vector<double> >();	
		}

	} catch (const std::exception & e) {
		//std::cout << e.what() << std::endl;
		return std::vector<std::vector<double> >();	 // Pairwise tie encountered.
	}

	// 3. Get the election (permutation) vectors A, B, C, D, A', B', C', D'.

	std::vector<std::vector<double> > base_election_different_cands,
		modified_election_different_cands;

	for (int i = 0; i < numcands; ++i) {
		// TODO: Multiple cand permutations may exist. Handle it.
		std::vector<int> center_on_this_cand = candidate_remappings[i].
			find(desired_scenario)->second.cand_permutations[0];

		base_election_different_cands.push_back(
			get_ballot_vector(permute_election_candidates(
				base_election, center_on_this_cand), numcands));
		modified_election_different_cands.push_back(
			get_ballot_vector(permute_election_candidates(
				alteration.second, center_on_this_cand), numcands));
	}

	// 4. For each generic function:
	//		4.1. Record its score on A, B, C, D, A', B', C', D'.

	std::vector<std::vector<double> > results_all_functions;
	std::vector<double> results_this_function;

	gen_custom_function tester(numcands);

	for (algo_t algorithm : functions_to_test) {
		results_this_function.clear();
		assert(tester.set_algorithm(algorithm));

		for (int i = 0; i < numcands; ++i) {
			results_this_function.push_back(tester.evaluate(
				base_election_different_cands[i]));
		}

		for (int i = 0; i < numcands; ++i) {
			results_this_function.push_back(tester.evaluate(
				modified_election_different_cands[i]));
		}

		results_all_functions.push_back(results_this_function);
	}

	return results_all_functions;

	// 5. Go to 1 until we have enough data points.

	// 7. Do the filtering (more stuff to come.)
}

std::vector<std::vector<std::vector<std::vector<double> > > > test_many_times(
	int maxiter, const copeland_scenario & desired_scenario, int numcands,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
	candidate_remappings, const std::vector<algo_t> & functions_to_test) {

	int seed = 99;
	rng randomizer(seed);

	srand(seed);
	srandom(seed);
	srand48(seed);


	// The return vector's format is out[i][j][k][l]:
	//		the ith voting method's
	//		result on the lth test
	//		either before (j=0) or after (j=1) the improvement of 
	//			A's condition
	//		from the perspective of the kth candidate

	// score(method, A) - score(method, B) 
	//		<= score(method, A') - score(method, B')

	std::vector<std::vector<std::vector<std::vector<double> > > > out(
		functions_to_test.size(), 
		std::vector<std::vector<std::vector<double> > >(2, 
			std::vector<std::vector<double> >(numcands)));

	for (int iter = 0; iter < maxiter; ++iter) {

		std::vector<std::vector<double> > one_test_round = 
			test(desired_scenario, numcands, candidate_remappings,
				functions_to_test, randomizer);

		if (one_test_round.size() == 0) {
			--iter;
			continue;
		} else {
			cout << iter/(double)maxiter << "    \r" << flush;
		}

		for (int funct_idx = 0; funct_idx < functions_to_test.size(); 
			++funct_idx) {

			// Before-elections from the various candidates' perspectives.
			for (int candidate_idx = 0; candidate_idx < numcands; 
				++candidate_idx) {
				out[funct_idx][0][candidate_idx].push_back(
					one_test_round[funct_idx][candidate_idx]);
			}

			for (int candidate_idx = 0; candidate_idx < numcands; 
				++candidate_idx) {
				out[funct_idx][1][candidate_idx].push_back(
					one_test_round[funct_idx][candidate_idx+numcands]);
			}
		}
	}

	return out;
}

std::vector<double> subtract(const std::vector<double> & a, 
	const std::vector<double> & b) {

	std::vector<double> result;
	std::transform(a.begin(), a.end(), b.begin(),
		std::back_inserter(result), std::minus<int>());

	return result;
}

// Returns true if no element in a is greater than the corresponding element
// in b, and at least one element in a is less than the corresponding element
// in b.

bool dominated_less(const std::vector<double> & a,
	const std::vector<double> & b) {

	bool one_less = false;

	for (int i = 0; i < std::min(a.size(), b.size()); ++i) {
		if (a[i] > b[i]) { return false; }
		one_less |= (a[i] < b[i]);
	}

	return one_less;
	//return true;
}

// Returns true if no element in a is above zero while the corresponding
// element is below zero in b, and at least one of the elements below zero
// in a has a corresponding element above zero in b.

// The point of this is that we don't care if b's score gets closer to a's
// score after we do something that benefits a; only that a doesn't go from
// being superior to b to being inferior to b when something that should 
// benefit a occurs.

// Remember to do enough tests in one go that one_less will be true for
// methods that pass the monotonicity criterion in question!
bool dominated_margin_less(const std::vector<double> & a,
	const std::vector<double> & b) {

	bool one_less = false;
	bool one_below = false;

	for (int i = 0; i < std::min(a.size(), b.size()); ++i) {
		if (a[i] > 0 && b[i] < 0) { return false; }
		one_less |= (a[i] < 0) && (b[i] >= 0);
		one_below |= a[i] < 0;
	}

	if (!one_below)
		return true; // a never won compared to b to begin with
				// so we can't see if a benefitted from the
				// change. Let it pass.

	return one_less;
}

main() {
	std::map<copeland_scenario, isomorphism> foo = 
	get_derived_scenario_reductions(4);

	std::vector<std::map<copeland_scenario, isomorphism> > bar =
		get_candidate_remappings(4, foo);

	std::set<copeland_scenario> nonderived_full = get_nonderived_scenarios(4,
		foo);

	copeland_scenario example_desired;

	// I don't know why references don't work here...
	for (copeland_scenario x: nonderived_full) {
		std::cout << "Smith set 4 nonderived: " << x.to_string() << std::endl;
		example_desired = x;
	}

	// Testing get_ballot_vector.
	ballot_group abcd;
	abcd.complete = true;
	abcd.weight = 1;
	abcd.contents.insert(candscore(0, 10));
	abcd.contents.insert(candscore(1, 9));
	abcd.contents.insert(candscore(2, 8));
	abcd.contents.insert(candscore(3, 7));

	std::list<ballot_group> abcd_election;
	abcd_election.push_back(abcd);

	std::vector<double> abcd_only = get_ballot_vector(abcd_election, 4);

	std::copy(abcd_only.begin(), abcd_only.end(), 
		std::ostream_iterator<double>(std::cout, " "));

	std::cout << std::endl; // should be 1 first everything else 0

	ballot_group bca;
	bca.complete = true;
	bca.weight = 1.9;
	bca.contents.insert(candscore(1, 10));
	bca.contents.insert(candscore(2, 8));
	bca.contents.insert(candscore(0, 7));

	std::list<ballot_group> bca_election;
	bca_election.push_back(bca);

	std::vector<double> bca_only = get_ballot_vector(bca_election, 3);

	// Should be 0 0 1.9 0 0
	std::copy(bca_only.begin(), bca_only.end(), 
		std::ostream_iterator<double>(std::cout, " "));

	std::cout << std::endl;

	// Test permuting.

	list<ballot_group> abc_election = permute_election_candidates(
		bca_election, {1, 2, 0});

	std::vector<double> abc_only = get_ballot_vector(abc_election, 3);

	std::copy(abc_only.begin(), abc_only.end(), 
		std::ostream_iterator<double>(std::cout, " "));

	std::cout << std::endl; // should be 1.9 first everything else 0

	int seed = 97;
	rng randomizer(seed);


	srand(seed);
	srandom(seed);
	srand48(seed);


	std::vector<std::vector<double> > test_results_one = 
		test(example_desired, 4, bar, {104180872604, 104180877788}, 
			randomizer);

	for (int i = 0; i < test_results_one.size(); ++i) {
		std::cout << "Results for method number " << i << std::endl;
		std::copy(test_results_one[i].begin(), test_results_one[i].end(),
			ostream_iterator<double>(cout, " "));
		std::cout << std::endl;
	}

	/*for (int iter = 0; iter < 1000000; ++iter) {

		show_transitions(example_desired, 4, bar, randomizer);
	}*/

	/*
	{146105669084,
146106042332,
146106378647,
146106420835,
146108235235,
146109619573,
146109624755,
146109624757,
146109629939,
146109629941}
*/


	std::vector<algo_t> prospective_functions({146105669084,
		5535781,
5545580,
5545930,
5551332,
5603533,
5608717,
5613901,
90668300,
90668372,
90668443,
90668444,
90668515,
90668516,
90668587,
90668588,
90668659,
90668660,
90668731,
90668732,
90668803,
90668804,
90668875,
90668876,
90668947,
90668948,
90669019,
90669020,
90669091,
90669092,
5686475,
146106042332,
146106378647,
146106420835,
146108235235,
146109619573,
146109624755,
146109624757,
146109629939,
146109629941, 1, 25, 54, 133, 205, 277, 349, 
		421, 493, 565});

	std::vector<std::vector<std::vector<std::vector<double> > > > 
		many_test_results = test_many_times(20000, example_desired, 4,
			bar, prospective_functions);

	for (int method_one = 0; method_one < prospective_functions.size(); ++method_one) {
		// Is some kind of sorting possible here? Yeah, it is, but I can't be
		// bothered to write the algorithm at the moment. Something like: have a
		// list of references to the method sorted by first element, then another
		// list sorted by second element, and so on...
		for (int method_two = 0; method_two < prospective_functions.size(); ++method_two) {

			// score(method, A) - score(method, B) 
			//		<= score(method, A') - score(method, B')

			std::vector<double> before_margin = subtract(many_test_results[method_one][0][0], many_test_results[method_two][0][1]);
			std::vector<double> after_margin = subtract(many_test_results[method_one][1][0], many_test_results[method_two][1][1]);

			if (dominated_margin_less(before_margin,after_margin)) {
				std::cout << "Might be so: " << prospective_functions[method_one] << ", " << prospective_functions[method_two] << std::endl;
				/*std::cout << "Margin before: ";
				std::copy(before_margin.begin(), before_margin.begin()+20,
					ostream_iterator<double>(cout, " "));
				std::cout << std::endl;
				std::cout << "Margin after: ";
				std::copy(after_margin.begin(), after_margin.begin()+20,
					ostream_iterator<double>(cout, " "));
				std::cout << std::endl;*/
				for (int method_three = 0; method_three < prospective_functions.size(); ++method_three) {
					before_margin = subtract(many_test_results[method_one][0][0], many_test_results[method_three][0][2]);
					after_margin = subtract(many_test_results[method_one][1][0], many_test_results[method_three][1][2]);					

					if (dominated_less(before_margin,after_margin)) {
						std::cout << "Three pass: " << prospective_functions[method_one] << " " << prospective_functions[method_two] << " " << prospective_functions[method_three] << std::endl;
					}
				}
			}
		}
	}
}
