
// Some actual composition testing.

// TODO: Rewrite these comments.

#include "equivalences.cc"
#include "eligibility.h"


// We need a function that generates a monotonicity election pair using some
// monotonicity test, with the restriction that A should be the one who
// benefits.

// Then we need a function that runs all four rotations for that election
// on all the custom_functs that are still eligible for the scenario in
// question, and updates the eligibility tables.

// The we need a function that goes through all the eligible pairs and
// combines them into eligible four-tuples.

// Perhaps the other way around?
struct election_scenario_pair {
	std::list<ballot_group> election;
	copeland_scenario scenario;
};

struct permuted_election_vector {
	// This variable is set if the election/scenario pair arose from
	// permuting another election so some candidate is now the first
	// candidate in the permuted election. In that case, this value is
	// the index of the candidate (e.g. 1 for B); if there's no such
	// permutation, this value is 0.
	int permuted_to_candidate;

	std::vector<double> election;
};


struct monotonicity_pair {
	election_scenario_pair base_election;
	election_scenario_pair improved_election;
};

monotonicity_pair get_monotonicity_pair(const copeland_scenario & 
	desired_scenario, int numcands, rng & randomizer, monotonicity *
	mono_test) {

	impartial ic(true);
	monotonicity_pair out;

	condmat condorcet_matrix(CM_PAIRWISE_OPP);

	bool succeeded = false;

	while (!succeeded) {

		// Get the base election

		do {
			// Use an odd number of voters to avoid ties.
			out.base_election.election = ic.generate_ballots(
				2 * randomizer.lrand(5, 50) + 1, numcands, randomizer);
			condorcet_matrix.zeroize();
			condorcet_matrix.count_ballots(out.base_election.election, 
				numcands);
			out.base_election.scenario = copeland_scenario(&condorcet_matrix);
		} while (out.base_election.scenario != desired_scenario);

		// Create data (specs) for the monotonicity test and make it prefer 
		// candidate 0 (A).

		std::vector<int> mono_data = mono_test->generate_aux_data(
			out.base_election.election, numcands);

		mono_test->set_candidate_to_alter(mono_data, 0);

		std::pair<bool, list<ballot_group> > alteration = mono_test->
			rearrange_ballots(out.base_election.election, numcands, 
				mono_data);

		// If we didn't succeed, loop back to start.
		if (!alteration.first) {
			continue;
		}

		out.improved_election.election = alteration.second;

		// Check for ties and set the improved scenario (kinda ugly)
		try {
			condorcet_matrix.zeroize();
			condorcet_matrix.count_ballots(out.improved_election.election,
				numcands);
			out.improved_election.scenario = copeland_scenario(
				&condorcet_matrix);
		} catch (const std::exception & e) { 
			continue;
		}

		// If we get here, we have an election example that we can use.
		succeeded = true;
	}

	return out;
}

// This gets the election results rotated for every candidate. Note that
// there may be more elements in the list than there are candidates, if
// there's more than one rotation for a particular candidate. Thus the
// return value makes no guarantee that the second entry is the perspective
// from the point of view of B. 

std::list<permuted_election_vector> get_all_permuted_elections(
	const election_scenario_pair & cand_As_perspective,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
	candidate_remappings, int numcands) {

	std::list<permuted_election_vector> out;

	for (int i = 0; i < numcands; ++i) {
		// TODO: Multiple cand permutations may exist. Handle it.
		for (const auto & cand_permutation: candidate_remappings[i].
			find(cand_As_perspective.scenario)->second.cand_permutations) {

			permuted_election_vector pev;
			pev.permuted_to_candidate = i;
			pev.election = get_ballot_vector(permute_election_candidates(
				cand_As_perspective.election, cand_permutation), numcands);

			out.push_back(pev);
		}
	}

	return out;
}


// Testing testing

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

	mono_add_top mattest(false, false);
	mono_raise mrtest(false, false);

	monotonicity * mono_test = &mattest;
	if (randomizer.drand() < 0.5) mono_test = &mrtest;

	// - Get a monotonicity pair with the test we decided to use.

	monotonicity_pair mono_pair = get_monotonicity_pair(desired_scenario,
		numcands, randomizer, mono_test);
	
	// - Get the election (permutation) vectors A, B, C, D, A', B', C', D'.

	std::list<permuted_election_vector> permuted_base_election = 
		get_all_permuted_elections(mono_pair.base_election,
			candidate_remappings, numcands);

	std::list<permuted_election_vector> permuted_improved_election = 
		get_all_permuted_elections(mono_pair.improved_election,
			candidate_remappings, numcands);
	
	std::vector<std::vector<double> > base_election_different_cands,
		modified_election_different_cands;

	for (int i = 0; i < numcands; ++i) {
		// Teh hax
		// Needs a more thorough refactoring later.
		std::vector<double> base;
		for (const permuted_election_vector & pie: permuted_base_election) {
			if (pie.permuted_to_candidate == i) {
				base = pie.election;
			}
		}
		base_election_different_cands.push_back(base);

		std::vector<double> imp;
		for (const permuted_election_vector & pie: 
			permuted_improved_election) {

			if (pie.permuted_to_candidate == i) {
				imp = pie.election;
			}
		}
		modified_election_different_cands.push_back(imp);
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

/*void test_against_eligibility(eligibility_tables & eligibilities,
	const copeland_scenario & desired_scenario, int numcands,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
	candidate_remappings, const std::vector<algo_t> & functions_to_test,
	rng & randomizer, bool first_round) {

	// Ad hoc testing scheme. Improve later.

	mono_add_top mattest(false, false);
	mono_raise mrtest(false, false);

	monotonicity * mono_test = &mattest;
	if (randomizer.drand() < 0.5) mono_test = &mrtest;

	// - Get a monotonicity pair with the test we decided to use.

	monotonicity_pair mono_pair = get_monotonicity_pair(desired_scenario,
		numcands, randomizer, mono_test);
	
	// - Get the election (permutation) vectors A, B, C, D, A', B', C', D'.

	std::list<permuted_election_vector> permuted_base_election = 
		get_all_permuted_elections(mono_pair.base_election,
			candidate_remappings, numcands);

	std::list<permuted_election_vector> permuted_improved_election = 
		get_all_permuted_elections(mono_pair.improved_election,
			candidate_remappings, numcands);
	
	std::vector<std::vector<double> > base_election_different_cands,
		modified_election_different_cands;

	// First get results for all functions for A. (Later: should check this
	// against every function that's in the union of eligibles_kth_column
	// for all columns. Later later, should just memoize.)

	// Let's do it the easy way for now, with all functions for all
	// elections, and then we just mark_eligible those who are still
	// eligible.

	gen_custom_function tester(numcands);

	// A

	std::vector<double> results_for_a, results_for_amark;

	assert(permuted_base_election.begin()->permuted_to_candidate == 0);
	assert(permuted_improved_election.begin()->permuted_to_candidate == 0);

	for (algo_t algorithm : functions_to_test) {
		assert(tester.set_algorithm(algorithm));

		results_for_a.push_back(tester.evaluate(
			permuted_base_election.begin()->election));
		results_for_amark.push_back(tester.evaluate(
			permuted_improved_election.begin()->election));
	}

	// Now for every other candidate...
	for (algo_t algorithm : functions_to_test) {
		for (int cand = 1; cand < numcands; ++cand) {

			std::vector<double> base;
			for (const permuted_election_vector & pie: permuted_base_election) {
				if (pie.permuted_to_candidate == i) {
					base = pie.election;
				}
			}

			std::vector<double> imp;
			for (const permuted_election_vector & pie: 
				permuted_improved_election) {

				if (pie.permuted_to_candidate == i) {
					imp = pie.election;
				}
			}

			assert(tester.set_algorithm(algorithm));

			double base_result = tester.evaluate(base),
				imp_result = tester.evaluate(imp);

			// for every other function/algorithm
			for (size_t other_algorithm_idx = 0; 
					other_algorithm_idx < functions_to_test.size();
					++other_algorithm_idx) {

				// score(A) - score(B)
				double margin_before = results_for_a[other_algorithm_idx] -
					base_result;
				double margin_after = results_for_amark[other_algorithm_idx]
					- imp_result;

				// If it's the first round, then mark eligible every 
				// combination with margin_before <= margin_after. Otherwise,
				// mark as ineligible every combination with margin_before >
				// margin_after.
				// This needs to be handled in a different way, because we
				// may have an inconclusive result, so whether it's the first
				// round depends on who we're testing.
				if (first_round && margin_before <= margin_after) {
					// ...
				}
				if (!first_round && margin_before > margin_after) {
					eligibilities.table_per_scenario_tuple[scenario_tuple].
						mark_ineligible(functions_to_test[other_algorithm_idx],
							algorithm);
				}
		}
	}
}*/

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

		for (size_t funct_idx = 0; funct_idx < functions_to_test.size(); 
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

	for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
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

	for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
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

int main() {
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

	for (size_t i = 0; i < test_results_one.size(); ++i) {
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
		many_test_results = test_many_times(200, example_desired, 4,
			bar, prospective_functions);

	for (size_t method_one = 0; method_one < prospective_functions.size(); ++method_one) {
		// Is some kind of sorting possible here? Yeah, it is, but I can't be
		// bothered to write the algorithm at the moment. Something like: have a
		// list of references to the method sorted by first element, then another
		// list sorted by second element, and so on...
		for (size_t method_two = 0; method_two < prospective_functions.size(); ++method_two) {

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
				for (size_t method_three = 0; method_three < prospective_functions.size(); ++method_three) {
					before_margin = subtract(many_test_results[method_one][0][0], many_test_results[method_three][0][2]);
					after_margin = subtract(many_test_results[method_one][1][0], many_test_results[method_three][1][2]);					

					if (dominated_less(before_margin,after_margin)) {
						std::cout << "Three pass: " << prospective_functions[method_one] << " " << prospective_functions[method_two] << " " << prospective_functions[method_three] << std::endl;
					}
				}
			}
		}
	}

	return 0;
}
