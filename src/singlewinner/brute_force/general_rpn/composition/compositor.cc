
// Some actual composition testing.

// TODO: Rewrite these comments.

#include "equivalences.cc"


// We need a function that generates a monotonicity election pair using some
// monotonicity test, with the restriction that A should be the one who
// benefits.

// Then we need a function that runs all four rotations for that election
// on all the custom_functs that are still eligible for the scenario in
// question, and updates the eligibility tables.

// The we need a function that goes through all the eligible pairs and
// combines them into eligible four-tuples.

struct monotonicity_pair {
	std::list<ballot_group> base_election;
	std::list<ballot_group> improved_election;
	copeland_scenario base_scenario, improved_scenario;
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
			out.base_election = ic.generate_ballots(
				2 * randomizer.lrand(5, 50) + 1, numcands, randomizer);
			condorcet_matrix.zeroize();
			condorcet_matrix.count_ballots(out.base_election, numcands);
			out.base_scenario = copeland_scenario(&condorcet_matrix);
		} while (out.base_scenario != desired_scenario);

		// Create data (specs) for the monotonicity test and make it prefer 
		// candidate 0 (A).

		std::vector<int> mono_data = mono_test->generate_aux_data(
			out.base_election, numcands);

		mono_test->set_candidate_to_alter(mono_data, 0);

		std::pair<bool, list<ballot_group> > alteration = mono_test->
			rearrange_ballots(out.base_election, numcands, mono_data);

		// If we didn't succeed, loop back to start.
		if (!alteration.first) {
			continue;
		}

		out.improved_election = alteration.second;

		// Check for ties and set the improved scenario (kinda ugly)
		try {
			condorcet_matrix.zeroize();
			condorcet_matrix.count_ballots(out.improved_election, numcands);
			out.improved_scenario = copeland_scenario(&condorcet_matrix);
		} catch (const std::exception & e) { 
			continue;
		}

		// If we get here, we have an election example that we can use.
		succeeded = true;
	}

	return out;
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
		many_test_results = test_many_times(20000, example_desired, 4,
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
