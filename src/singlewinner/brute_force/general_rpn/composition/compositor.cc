
// Some actual composition testing.

// TODO: Rewrite these comments.
// TODO: Make it catch and remove (X, Y) combinations where Y is a constant
// function but X passes mono-add-top or mono-raise with every other function.
// As it is, the responsivity of X will make Y seem responsive as well, i.e.
// if score(X, A) < score(X, A'), then if score(B, p) == c for all p,
// score(X,A) - score(Y, B) <= score(X,A') - score(Y, B') and so will
// currently pass even though Y is worthless.

// Problems so far: Need to make sure that B and Bprime put the same
// candidate as candidate 0. Put that inside get_test_instance or as a
// separate function? How will it interface with the ISDA reduction?
// Actually, given A, B, and A', we have forced B' under the constraint.
// It can only be a particular scenario. Make a function to find out what
// scenario it will be.

// Need to make get_test_instance faster when A' is to have 3 candidates
// and A has a 4-cddt Smith set.

#include "equivalences.cc"
#include "eligibility.h"
#include "../isda.cc"

#include <time.h>

// Indices to the four different scenarios for monotonicity checking:
// A (the initial election), B (the initial election from the POV of
// someone else), A' (the election after A's condition has been improved),
// and B' (A' from the perspective of whoever B's perspective was from)
/*const int ELEC_A = 0,
		  ELEC_B = 1,
		  ELEC_APRIME = 2,
		  ELEC_BPRIME = 3,
		  ELEC_NUM = 4;
TODO: Implement this. */

// We need a function that generates a monotonicity election pair using some
// monotonicity test, with the restriction that A should be the one who
// benefits.

// Then we need a function that runs all four rotations for that election
// on all the custom_functs that are still eligible for the scenario in
// question, and updates the eligibility tables.

// The we need a function that goes through all the eligible pairs and
// combines them into eligible four-tuples.

// Given an election-scenario pair, turn the election into the
// desired scenario by permuting the candidates. The function
// throws an exception if permuting to that scenario is impossible.
// If consider_A is false, we only look at permutations that map some
// other candidate to A (this is useful for monotonicity).

// If desired_candidate is -1, then it will try to find something from
// the perspective of any candidate; otherwise, it will only consider
// that particular candidate.

// TODO: Investigate why we can't remap derived scenarios to canonical
// ones.

election_scenario_pair permute_to_desired(election_scenario_pair cur,
	const copeland_scenario & desired_scenario, int desired_candidate,
	bool consider_A,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
		candidate_remappings) {

	if (cur.scenario == desired_scenario && desired_candidate == 0) {
		return cur;
	}

	size_t numcands = cur.scenario.get_numcands();

	for (size_t i = 0; i < numcands; ++i) {
		if (desired_candidate != -1 && i != (size_t)desired_candidate) { 
			continue; 
		}
		if (i == 0 && !consider_A) { continue; }

		if (candidate_remappings[i].find(cur.scenario) ==
			candidate_remappings[i].end()) {
			throw std::runtime_error(
					"permute_to_desired: could not find source scenario!");
		}

		isomorphism dest_isomorphism = candidate_remappings[i].find(
			cur.scenario)->second;

		if (dest_isomorphism.to_scenario != desired_scenario) {
			continue;
		}

		cur.election = permute_election_candidates(cur.election,
			*dest_isomorphism.cand_permutations.begin());

		cur.scenario = dest_isomorphism.to_scenario;
		cur.from_perspective_of = i;

		assert (cur.from_perspective_of == desired_candidate || desired_candidate == -1);

		return cur;
	}

	throw std::runtime_error(
		"permute_to_desired: could not remap " + cur.scenario.to_string() +
		" to " + desired_scenario.to_string());
}

// Permute the given election-scenario pair so that desired_candidate is now
// candidate 0.
// NOTE: The function below only works if cur.from_perspective_of is 0
// because otherwise the election_scenario_pair is the result of some
// permutation, and you should call this with whatever was the source of that
// permutation instead. This function will raise an exception if
// cur.from_perspective differs from zero.

election_scenario_pair permute_to_desired(election_scenario_pair cur,
	int desired_candidate,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
	candidate_remappings) {

	if (desired_candidate == 0)
		return cur;

	if (cur.from_perspective_of != 0) {
		throw std::runtime_error(
			"permute_to_desired: nested remapping not allowed");
	}

	if (candidate_remappings[desired_candidate].find(cur.scenario) ==
		candidate_remappings[desired_candidate].end()) {

		throw std::runtime_error(
			"permute_to_desired: could not find source scenario!");
	}

	isomorphism dest_isomorphism = candidate_remappings[desired_candidate].
		find(cur.scenario)->second;

	cur.election = permute_election_candidates(cur.election,
		*dest_isomorphism.cand_permutations.begin());

	cur.scenario = dest_isomorphism.to_scenario;
	cur.from_perspective_of = desired_candidate;

	return cur;
}

// Apply ISDA to an election_scenario_pair.

election_scenario_pair apply_ISDA(const election_scenario_pair & in) {
	reduction ISDA_reduction = get_ISDA_reduction(in.scenario);

	// If ISDA doesn't change the number of candidates (i.e. we have a
	// full Smith set), just directly return the input.

	size_t reduced_numcands = ISDA_reduction.to_scenario.get_numcands();
	if (reduced_numcands == in.scenario.get_numcands()) { return in; }

	// Otherwise, reduce.

	election_scenario_pair out;

	// Should be verified, rather.
	// out.scenario = ISDA_reduction.to_scenario;

	out.election = relabel_election_candidates(in.election,
		ISDA_reduction.cand_relabeling, false);
	// From_perspective_of is relative to the original election, which
	// hasn't been altered (or if it has, we don't know it). So keep the
	// from_perspective_of.
	out.from_perspective_of = in.from_perspective_of;
	out.scenario = copeland_scenario(out.election, reduced_numcands);

	// Assert that the scenario matches.
	assert(out.scenario == ISDA_reduction.to_scenario);
	// And assert that what used to be A in the input pair was not
	// eliminated (because then ISDA dictates the score of the output
	// election should be -infinity from the point of view of A).

	assert(find(ISDA_reduction.cand_relabeling.begin(),
		ISDA_reduction.cand_relabeling.end(), 0) !=
		ISDA_reduction.cand_relabeling.end());

	return out;
}

election_scenario_pair apply_canonical_ISDA(
	const election_scenario_pair & in,
	const std::map<int, fixed_cand_equivalences> & canonicalization) {

	// First apply a reduction to (a potentially noncanonical) scenario.
	election_scenario_pair out = apply_ISDA(in);
	size_t out_numcands = out.scenario.get_numcands();

	// Then further transform to canonical.
	isomorphism canonical_isomorphism = canonicalization.find(out_numcands)->
		second.get_canonical_isomorphism(out.scenario);

	// This was taken from permute_to_desired. Refactor?
	// Should probably be in the fixed_cand_equivalences class.
	out.election = permute_election_candidates(out.election,
			canonical_isomorphism.cand_permutations[0]);

	out.scenario = canonical_isomorphism.to_scenario;

	return out;
}


// Given scenarios A, A', and B, determine what scenario B' must
// necessarily be.

// To handle reversed situations properly (see below for what that is),
// we need to know the proper scenarios for A, A', B, and B'.
copeland_scenario get_B_prime_scenario(
	const copeland_scenario & A_scenario,
	const copeland_scenario & B_scenario, int B_candidate,
	const copeland_scenario & A_prime_scenario,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
		candidate_remappings) {

	// Make a phantom election_scenario_pair corresponding to A.
	election_scenario_pair A;
	A.scenario = A_scenario;
	A.from_perspective_of = 0;

	// This gives us the candidate that corresponds to the B scenario.
	election_scenario_pair B = permute_to_desired(A, B_scenario, 
		B_candidate, false, candidate_remappings);

	// Now permute A' to the same candidate...
	election_scenario_pair A_prime = A;
	A_prime.scenario = A_prime_scenario;

	election_scenario_pair B_prime = permute_to_desired(A_prime,
		B.from_perspective_of, candidate_remappings);

	// and get the corresponding scenario.
	return B_prime.scenario;
}

// This is a fully completed test instance. The pair (X, Y) assigned
// to the scenarios of before_A and before_B respectively (with after_*
// having the same scenarios as their before_* counterparts) pass if
// score(before_A, X) - score(before_B, Y) <= 0 and
// score(after_A, X) - score(after_B, y) >= 0. If both have the same sign,
// we learn nothing, and if it's > and <= or >= and <, then it fails.

// Note thus that "before_B" does not have to be the before_A ballot with
// candidates relabeled so that what was B is now A. It could just as easily
// be with C relabeled to A, or a reverse transformation (see below).

struct monotonicity_test_instance {
	election_scenario_pair before_A, before_B, after_A, after_B;
};

// This function takes in a scenario where before_A is assigned to scenario
// sA and before_B is assigned to scenario sB, and produces a valid
// monotonicity test instance with sA and sB flipped, so that its before_A
// now has sB and its before_B now has sA. The purpose is to reduce the
// problem of monotonicity testing pairs with X assigned to sA and Y assigned
// to sB with sA < sB to the problem of testing with sA > sB.

// Suppose in has sA for before_A and sB for before_B. Then a method pair
// (X, Y) passes if score(before_A, X) - score(before_B, Y) <= 0 and
// score(after_A, X) - score(after_B, Y). Multiply this by -1 to get
// score(before_B, Y) - score(before_A, X) >= 0 and
// score(after_B, Y) - score(after_A, X) <= 0. In other words, to reverse
// the order of the scenarios while keeping the pass test the same, all we
// have to do is to turn
//  in: after_B    after_A     before_B   before_A	to
// out: before_A   before_B    after_A    after_B

monotonicity_test_instance reverse_transform(const
	monotonicity_test_instance & in) {

	monotonicity_test_instance out;

	out.before_A = in.after_B;
	out.before_B = in.after_A;
	out.after_A = in.before_B;
	out.after_B = in.before_A;

	return out;
}

// Get a test instance with the desired scenarios. If it finds anything,
// returns true and sets out. Otherwise returns false, and the contents of
// out are undefined.

// I'm feeling like ISDA should be a postprocessing stage. For four cands,
// we let desired_A_scenario (or desired_B_scenario) be one with a Smith
// set size three. All we need to make sure of is that the candidate B is
// in the Smith set, which we can do by appropriately specifying
// desired_B_scenario. get_test_instance will be slower than it need to be
// because reduction might make B nor matter (e.g. it doesn't matter who
// the Smith loser is when going 4->3 as long as B is not it, because every
// three-cycle leads to ABCA with appropriate relabeling anyway). But the
// advantage of doing it that way is that I can actually get the method
// done without too much hassle; then I can optimize later if necessary.

// Hotspot.
bool get_test_instance(const copeland_scenario * desired_A_scenario,
	const copeland_scenario * desired_B_scenario, 
	int desired_B_candidate, 
	const copeland_scenario * desired_Aprime_scenario,
	const fixed_cand_equivalences & equivalences,
	const std::map<int, fixed_cand_equivalences> & other_equivalences,
	int numcands, rng & randomizer, monotonicity * mono_test, bool reverse,
	bool do_apply_ISDA, monotonicity_test_instance & out) {

	/*std::cout << std::endl;
	std::cout << "gti1: A: " << desired_A_scenario->to_string() << std::endl;
	std::cout << "gti1: B: " << desired_B_scenario->to_string() << std::endl;
	std::cout << "gti1: Ap: " << desired_Aprime_scenario->to_string() << std::endl;
	*/
	//std::cout << "." << std::flush;

	impartial ic(false);

	election_scenario_pair before, after;

	int num_ballots;

	// Get the B prime scenario.
	copeland_scenario B_prime = get_B_prime_scenario(
		*desired_A_scenario, *desired_B_scenario, desired_B_candidate, 
		*desired_Aprime_scenario, equivalences.get_cand_remaps());

	const copeland_scenario * desired_Bprime_scenario = &B_prime;

	// If we're reversed, we want to first generate something with
	// desired_Bprime_scenario for the A election (and desired_Aprime_scenario
	// for the B election), and then swap them around using
	// reverse_transform. So in that case what the user specified as
	// desired_A_transform should internally be desired_Bprime_transform and
	// vice versa.
	if (reverse) {
		std::cout << "reverse" << std::endl;
		std::swap(desired_A_scenario, desired_Bprime_scenario);
		std::swap(desired_B_scenario, desired_Aprime_scenario);
	}

	// Generate an acceptable ballot. We only need a scenario that we can
	// turn into the deisred scenario by relabeling candidates, because we
	// can relabel afterwards if the scenario doesn't exactly match.

	/*std::cout << std::endl;
	std::cout << "gti: A: " << desired_A_scenario->to_string() << std::endl;
	std::cout << "gti: B: " << desired_B_scenario->to_string() << std::endl;
	std::cout << "gti: Ap: " << desired_Aprime_scenario->to_string() << std::endl;
	std::cout << "gti: Bp: " << desired_Bprime_scenario->to_string() << std::endl;
	*/

	do {
		// Use an odd number of voters to avoid ties.
		num_ballots = 2 * randomizer.lrand(5, 50) + 1;
		before.election = ic.generate_ballots(num_ballots, numcands,
			randomizer);
		before.scenario = copeland_scenario(before.election, numcands);
	} while (!equivalences.is_transformable_into(before.scenario,
		*desired_A_scenario));

	// Permute into the A_scenario we want.
	before = permute_to_desired(before, *desired_A_scenario, -1, true,
		equivalences.get_cand_remaps());
	// Whatever the permutation is, what we end up with is A's perspective
	// (by definition, because it has desired_A_scenario as scenario). So
	// make that clear.
	before.from_perspective_of = 0;

	// Create data (specs) for the monotonicity test and make it prefer
	// candidate 0 (A). We may fail to alter the ballot the way we want,
	// so all of this is in a loop; if the before ballot is already good
	// enough, there's no need to waste it.

	bool found_improved = false;
	for (int i = 0; i < 10 && !found_improved; ++i) {
		std::vector<int> mono_data = mono_test->generate_aux_data(
			before.election, numcands);

		// TODO: Really need to rename this function...
		mono_data = mono_test->set_candidate_to_alter(mono_data, 0);

		// Specify that we're only going to add one ballot in case of mono-
		// add-top (this makes it easier to get to an improved ballot set
		// that doesn't change the scenario).
		// TODO: Geometric distribution or something? Everything that gives
		// us wider coverage is good.
		int num_to_add = randomizer.lrand(2, num_ballots);
		std::pair<bool, list<ballot_group> > alteration = mono_test->
			rearrange_ballots(before.election, numcands, num_to_add,
				mono_data);

		// If we didn't succeed, bail.
		if (!alteration.first) {
			continue;
		}

		after.election = alteration.second;
		after.from_perspective_of = 0;

		// Check for ties and set the improved scenario (kinda ugly)
		try {
			after.scenario = copeland_scenario(after.election, numcands);
		} catch (const std::exception & e) {
			continue;
		}

		// Unless the after scenario is what we want, bail.
		if (after.scenario != *desired_Aprime_scenario) {
			continue;
		}

		found_improved = true;
	}

	// Giving up.
	if (!found_improved) {
		return false;
	}

	// Generate before_B and after_B

	out.before_A = before;
	out.after_A = after;
	out.before_B = permute_to_desired(out.before_A, *desired_B_scenario,
		desired_B_candidate, false, equivalences.get_cand_remaps());

	// We need to do it this way to preserve the candidate number. Using
	// the other permute_to_desire may fail to do so, e.g. suppose after_A
	// is a three-candidate three-cycle. Then every other candidate's
	// perspective is also a three-cycle, but only one of those perspectives
	// is what we want to have the same perspective as in after_A.
	out.after_B = permute_to_desired(out.after_A,
		out.before_B.from_perspective_of, equivalences.get_cand_remaps());

	// Make sure we have the proper scenarios and perspectives.
	assert(out.before_A.scenario == *desired_A_scenario);
	assert(out.before_B.scenario == *desired_B_scenario);
	assert(out.after_A.scenario == *desired_Aprime_scenario);
	assert(out.after_B.scenario == *desired_Bprime_scenario);

	assert(out.before_B.from_perspective_of == desired_B_candidate);

	assert(out.before_A.from_perspective_of ==
		out.after_A.from_perspective_of);
	assert(out.after_B.from_perspective_of ==
		out.before_B.from_perspective_of);

	if (reverse) {
		out = reverse_transform(out);
	}

	if (do_apply_ISDA) {
		out.before_A = apply_canonical_ISDA(out.before_A, other_equivalences);
		out.before_B = apply_canonical_ISDA(out.before_B, other_equivalences);
		out.after_A = apply_canonical_ISDA(out.after_A, other_equivalences);
		out.after_B = apply_canonical_ISDA(out.after_B, other_equivalences);
	}

	return true;
}

typedef float result_t;

struct test_results {
	copeland_scenario A_scenario, B_scenario, Aprime_scenario,
		Bprime_scenario;
	std::vector<std::vector<result_t> > A_before_results;
	std::vector<std::vector<result_t> > A_after_results;
	std::vector<std::vector<result_t> > B_before_results;
	std::vector<std::vector<result_t> > B_after_results;
};

test_results test_many(int numiters,
	const copeland_scenario & desired_A_scenario,
	const copeland_scenario & desired_B_scenario,
	int desired_B_candidate,
	const copeland_scenario & desired_Aprime_scenario,
	int numcands, const fixed_cand_equivalences & equivalences,
	const std::map<int, fixed_cand_equivalences> & other_equivalences,
	const std::vector<std::vector<algo_t> > & functions_to_test,
	rng & randomizer) {

	std::cout << "tm: A: " << desired_A_scenario.to_string() << std::endl;
	std::cout << "tm: B: " << desired_B_scenario.to_string() << std::endl;
	std::cout << "tm: Ap: " << desired_Aprime_scenario.to_string() << std::endl;

	// We guess the number of candidates for Bprime is equal to that of
	// Aprime, as it's just a candidate rotation.
	// HACK! Fix later!
	std::vector<size_t> numcands_per_election = {
		//4, 4, 3, 3
		3, 3, 4, 4
		/*desired_A_scenario.get_numcands(),
		desired_B_scenario.get_numcands(),
		desired_Aprime_scenario.get_numcands(),
		desired_Aprime_scenario.get_numcands()*/
	};

	std::copy(numcands_per_election.begin(), numcands_per_election.end(),
		ostream_iterator<size_t>(cout, " "));
	std::cout << std::endl;
	std::cout << functions_to_test[3].size() << std::endl;
	std::cout << functions_to_test[4].size() << std::endl;

	struct timespec last_time, next_time;

	clock_gettime(CLOCK_MONOTONIC, &last_time);

	test_results results;
	results.A_scenario = desired_A_scenario;
	results.B_scenario = desired_B_scenario;
	results.Aprime_scenario = desired_Aprime_scenario;
	results.Bprime_scenario = get_B_prime_scenario(
		desired_A_scenario, desired_B_scenario, desired_B_candidate,
		desired_Aprime_scenario, equivalences.get_cand_remaps());

	//size_t num_functions = functions_to_test.size(), i;

	// Ew. Fix later.
	results.A_before_results.resize(functions_to_test[
		numcands_per_election[0]].size());
	results.B_before_results.resize(functions_to_test[
		numcands_per_election[1]].size());
	results.A_after_results.resize(functions_to_test[
		numcands_per_election[2]].size());
	results.B_after_results.resize(functions_to_test[
		numcands_per_election[3]].size());

	monotonicity_test_instance test_instance;

	mono_add_top mattest(false, false);
	mono_raise mrtest(false, false);
	monotonicity * mono_test;

	std::vector<gen_custom_function> evaluators;
	size_t i;

	// evaluators[i] for i-candidate elections.
	for (i = 0; i < 5; ++i) {
		evaluators.push_back(gen_custom_function(std::max((size_t)1, i)));
	}

	int time_count = 0;
	int time_to_check = 1000;

	// Get maximum value for our progress indicator. Also declare the
	// variable for the non-resetting counter.
	size_t cur_count = 0;
	size_t max_count = 0;
	for (size_t eval_numcands = 3; eval_numcands < 5; ++eval_numcands) {
		max_count += functions_to_test[eval_numcands].size();
	}

	max_count *= numiters;

	for (int i = 0; i < numiters; ++i) {

		// Do we want mono-raise or mono-add-top? Decide with a coin flip.
		if (randomizer.drand() < 0.5) {
			//std::cout << "MR" << std::endl;
			mono_test = &mrtest;
		} else {
			//std::cout << "MAT" << std::endl;
			mono_test = &mattest;
		}

		// Mono-raise doesn't work yet, handle later.
		mono_test = &mattest;

		// Do we want a reverse test? Decide with a coin flip!

		/*bool reverse = randomizer.drand() < 0.5;

		std::cout << "INFINITY" << std::endl;
		std::cout << randomizer.drand() << std::endl;
		if (reverse) { std::cout << "reverse in " << std::endl; } else
		{ std::cout << "no reverse in " << std::endl; }*/

		// Run get_instance until we get a result that fits what we want.
		while (!get_test_instance(&desired_A_scenario, &desired_B_scenario,
			desired_B_candidate, &desired_Aprime_scenario, equivalences, 
			other_equivalences, numcands, randomizer, mono_test, false,
			/*reverse,*/ true, test_instance));

		//std::cout << "DONE" << std::endl;

		/*assert(test_instance.before_A.scenario == desired_A_scenario);
		assert(test_instance.before_B.scenario == desired_B_scenario);*/
		//assert(test_instance.after_A.scenario == desired_Aprime_scenario);

		// Prepare ballot vectors.
		std::vector<std::vector<double> > ballot_vectors = {
			get_ballot_vector(test_instance.before_A),
			get_ballot_vector(test_instance.before_B),
			get_ballot_vector(test_instance.after_A),
			get_ballot_vector(test_instance.after_B)
		};

		std::vector<size_t> numcands_per_election = {
			test_instance.before_A.scenario.get_numcands(),
			test_instance.before_B.scenario.get_numcands(),
			test_instance.after_A.scenario.get_numcands(),
			test_instance.after_B.scenario.get_numcands()
		};

		// Run test against everything, for each possible numcands choice.
		for (size_t eval_numcands = 3; eval_numcands < 5; ++eval_numcands) {
			//std::cout << "eval_numcands = " << eval_numcands << std::endl;
			for (size_t funct_idx = 0; funct_idx <
				functions_to_test[eval_numcands].size(); ++funct_idx) {

				// First determine if we're going to print a progress indicator.
				// Two steps to this: first a simple counter to not make checking
				// time happen too often (and thus steal too much CPU pwoer), and
				// then a check to see if at least a second has elapsed.
				if (++time_count == time_to_check) {
					clock_gettime(CLOCK_MONOTONIC, &next_time);

					double elapsed = (next_time.tv_sec - last_time.tv_sec);
					elapsed += (next_time.tv_nsec - last_time.tv_nsec) / 1e9;

					if (elapsed > 1) {
						std::cerr << (double)cur_count/max_count << "    \r" << flush;
						last_time = next_time;
					}

					time_count = 0;
				}

				++cur_count;

				assert(evaluators[eval_numcands].set_algorithm(
					functions_to_test[eval_numcands][funct_idx]));

				// Might have unfortunate interactions with reverse?
				// Nah, shouldn't be a problem.

				if(numcands_per_election[0] == eval_numcands) {
					results.A_before_results[funct_idx].push_back(
						evaluators[eval_numcands].evaluate(
							ballot_vectors[0]));
				}
				if(numcands_per_election[1] == eval_numcands) {
					results.B_before_results[funct_idx].push_back(
						evaluators[eval_numcands].evaluate(
							ballot_vectors[1]));
				}
				if(numcands_per_election[2] == eval_numcands) {
					results.A_after_results[funct_idx].push_back(
						evaluators[eval_numcands].evaluate(
							ballot_vectors[2]));
				}
				if(numcands_per_election[3] == eval_numcands) {
					results.B_after_results[funct_idx].push_back(
						evaluators[eval_numcands].evaluate(
							ballot_vectors[3]));
				}
			}
		}

		/*for (size_t j = 0; j < num_functions; ++j) {

			// First determine if we're going to print a progress indicator.
			// Two steps to this: first a simple counter to not make checking
			// time happen too often (and thus steal too much CPU pwoer), and
			// then a check to see if at least a second has elapsed.
			if (++time_count == time_to_check) {
				clock_gettime(CLOCK_MONOTONIC, &next_time);

				double elapsed = (next_time.tv_sec - last_time.tv_sec);
				elapsed += (next_time.tv_nsec - last_time.tv_nsec) / 1e9;

				if (elapsed > 1) {
					size_t current = i * num_functions + j,
						   maximum = numiters * num_functions;

					std::cerr << (double)current/maximum << "    \r" << flush;
					last_time = next_time;
				}

				time_count = 0;
			}

			assert(evaluator.set_algorithm(functions_to_test[j]));

			results.A_before_results[j].push_back(
				evaluator.evaluate(ballot_vectors[0]));
			results.B_before_results[j].push_back(
				evaluator.evaluate(ballot_vectors[1]));
			results.A_after_results[j].push_back(
				evaluator.evaluate(ballot_vectors[2]));
			results.B_after_results[j].push_back(
				evaluator.evaluate(ballot_vectors[3]));
		}*/
	}

	return results;
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

std::vector<result_t> subtract(const std::vector<result_t> & a,
	const std::vector<result_t> & b) {

	std::vector<result_t> result;
	std::transform(a.begin(), a.end(), b.begin(),
		std::back_inserter(result), std::minus<double>());

	return result;
}

// Returns true if no element in a is greater than the corresponding element
// in b, and at least one element in a is less than the corresponding element
// in b.

bool dominated_less(const std::vector<double> & a,
	const std::vector<double> & b) {

	bool one_less = false;

	for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
		if (isnan(a[i] || isnan(b[i]))) { return false; }
		if (a[i] > b[i]) { return false; }
		one_less |= (a[i] < b[i]);
	}

	return one_less;
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
bool dominated_margin_less(const std::vector<result_t> & a,
	const std::vector<result_t> & b) {

	bool one_less = false;
	bool one_below = false;

	for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
		if (isnan(a[i] || isnan(b[i]))) { return false; }
		if (a[i] > 0 && b[i] < 0) { return false; }
		one_less |= (a[i] < 0) && (b[i] >= 0);
		one_below |= a[i] < 0;
	}

	// TODO?? Better tie criterion? Count number of zeroes?
	if (!one_below)
		return true; // a never won compared to b to begin with
				// so we can't see if a benefitted from the
				// change. Let it pass.

	return one_less;
}

// TODO: This, but results.A_after_results[i], results.B_after_results[j]
// is 3 cddt and the other is 4.
void print_passing_pairs(const test_results & results,
	const std::vector<algo_t> & functions_tested) {

	std::cout << "Passing pairs for (" << results.A_scenario.to_string()
		<< ", " << results.B_scenario.to_string() << ")\n";

	for (size_t i = 0; i < functions_tested.size(); ++i) {
		for (size_t j = 0; j < functions_tested.size(); ++j) {
			std::vector<result_t> margin_before = subtract(
				results.A_before_results[i], results.B_before_results[j]);
			std::vector<result_t> margin_after = subtract(
				results.A_after_results[i], results.B_after_results[j]);

			if (dominated_margin_less(margin_before, margin_after)) {
				std::cout << "\tpass: " << functions_tested[i] << ", " <<
					functions_tested[j] << "\n";
			}
		}
	}
}

bool admissible(const test_results & results, size_t i, size_t j) {
	std::vector<result_t> margin_before = subtract(
		results.A_before_results[i], results.B_before_results[j]);
	std::vector<result_t> margin_after = subtract(
		results.A_after_results[i], results.B_after_results[j]);

	return dominated_margin_less(margin_before, margin_after);
}

bool admissible_four(const test_results & results, size_t i, size_t j,
	size_t k, size_t l) {

	std::vector<result_t> margin_before = subtract(
		results.A_before_results[i], results.B_before_results[j]);
	std::vector<result_t> margin_after = subtract(
		results.A_after_results[k], results.B_after_results[l]);

	return dominated_margin_less(margin_before, margin_after);
}


// Quick and dirty

void print_passing_tuples(const std::vector<std::vector<test_results> > &
	all_results, const std::vector<algo_t> & functions_tested) {

	// scenario 0	1	2	3
	// test     a   b   c   d

	for (size_t a = 0; a < functions_tested.size(); ++a) {
		for (size_t b = 0; b < functions_tested.size(); ++b) {
			if (!admissible(all_results[0][1], a, b)) continue;
			for (size_t c = 0; c < functions_tested.size(); ++c) {
				if (!admissible(all_results[1][2], b, b)) continue;
				if (!admissible(all_results[0][2], a, c)) continue;
				for (size_t d = 0; d < functions_tested.size(); ++d) {
					if (!admissible(all_results[2][3], c, d)) continue;
					if (!admissible(all_results[0][3], a, d)) continue;
					if (!admissible(all_results[1][3], b, d)) continue;

					std::cout << "PASS! " << functions_tested[a] << ", " <<
						functions_tested[b] << ", " << functions_tested[c] <<
						", " << functions_tested[d] << "\n";
				}
			}
		}
	}
}

// Quick and dirty, ditto
void print_passing_mixed_tuples(const test_results & results,
	const std::vector<std::vector<algo_t> > & functions_tested_per_candnum,
	const std::vector<size_t> & cands_per_election_type) {

	std::cout << "Passing tuples for (" << results.A_scenario.to_string()
		<< ", " << results.B_scenario.to_string() << ", "
		<< results.Aprime_scenario.to_string() << ", "
		<< results.Bprime_scenario.to_string() << std::endl;

	for (size_t Aidx = 0; Aidx < functions_tested_per_candnum[
		cands_per_election_type[0]].size(); ++Aidx) {

		algo_t A_algo = functions_tested_per_candnum[
			cands_per_election_type[0]][Aidx];
		for (size_t Bidx = 0; Bidx < functions_tested_per_candnum[
			cands_per_election_type[1]].size(); ++Bidx) {

			if (results.A_scenario == results.B_scenario &&
					Bidx != Aidx) {
					continue;
				}

			std::vector<result_t> margin_before = subtract(
					results.A_before_results[Aidx],
					results.B_before_results[Bidx]);

			algo_t B_algo = functions_tested_per_candnum[
				cands_per_election_type[1]][Bidx];

			for (size_t Ap_idx = 0; Ap_idx < functions_tested_per_candnum[
				cands_per_election_type[2]].size();

				++Ap_idx) {

				if (results.Aprime_scenario == results.A_scenario &&
					Ap_idx != Aidx) {
					continue;
				}

				if (results.Aprime_scenario == results.B_scenario &&
					Ap_idx != Bidx) {
					continue;
				}

				algo_t Ap_algo = functions_tested_per_candnum[
					cands_per_election_type[2]][Ap_idx];

				for (size_t Bp_idx = 0; Bp_idx < functions_tested_per_candnum[
					cands_per_election_type[3]].size();
					++Bp_idx) {

					if (results.Bprime_scenario == results.B_scenario &&
						Bp_idx != Bidx) {
						continue;
					}

					if (results.Bprime_scenario == results.Aprime_scenario &&
						Bp_idx != Ap_idx) {
						continue;
					}

					algo_t Bp_algo = functions_tested_per_candnum[
						cands_per_election_type[3]][Bp_idx];

					std::vector<result_t> margin_after = subtract(
						results.A_after_results[Ap_idx],
						results.B_after_results[Bp_idx]);

					if (dominated_margin_less(margin_before, margin_after)) {
						std::cout << "\tpass: " << A_algo << ", " <<
							B_algo << ", " << Ap_algo << ", " << Bp_algo <<
							"\n";
					}
				}
			}
		}
	}
}


int main(int argc, char ** argv) {

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	int numcands = 4;

	// Open some files to get back up to speed

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " [file containing sifter output, 3 cands] " <<
			"[file containing sifter output, 4 cands]" << std::endl;
		return(-1);
	}

	std::string filename = argv[1];
	std::ifstream sifter_file(filename);

	if (!sifter_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	std::vector<std::vector<algo_t> > prospective_functions(5);

	get_first_token_on_lines(sifter_file, prospective_functions[3]);

	sifter_file.close();

	// Cut and paste, whee

	filename = argv[2];
	sifter_file = std::ifstream(filename);

	if (!sifter_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	get_first_token_on_lines(sifter_file, prospective_functions[4]);

	sifter_file.close();

	fixed_cand_equivalences four_equivalences(4);

	std::map<int, fixed_cand_equivalences> other_equivs;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(4, fixed_cand_equivalences(4)));;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(3, fixed_cand_equivalences(3)));;


/*	std::map<copeland_scenario, isomorphism> scenario_reductions =
		get_derived_scenario_reductions(numcands);

	std::vector<std::map<copeland_scenario, isomorphism> > cand_remaps =
		get_candidate_remappings(numcands, scenario_reductions);*/

	std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(4,
		four_equivalences);

	std::vector<copeland_scenario> canonical_full_v;
	std::copy(canonical_full.begin(), canonical_full.end(),
		std::back_inserter(canonical_full_v));

	copeland_scenario example_desired_A, example_desired_B;

	// I don't know why references don't work here...
	for (copeland_scenario x: canonical_full) {
		std::cout << "Smith set 4 canonical: " << x.to_string() << std::endl;
	}

	example_desired_A = canonical_full_v[0];
	example_desired_B = canonical_full_v[1];

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

	// Test monotonicity generation with 3-scenario tests. Very rough
	// so far: need actual ISDA logic later.

	monotonicity_test_instance f;

	mono_add_top mat(false, false);
	mono_raise mr(false, false);

	std::cout << "Before get_instance" << std::endl;

	for (int i = 0; i < 0; ++i){

		copeland_scenario three_cand_smith_one(54, 4);
		// Can't be 5,4 because A is not in the Smith set in that scenario
		// and you can't push A off the Smith set by adding top or raising.
		// As it so happens, 54 is the only one where A is still in the
		// Smith set and the Smith set has size 3.

		if (get_test_instance(&canonical_full_v[2], &canonical_full_v[0],
			1, &three_cand_smith_one, four_equivalences, other_equivs, 4,
			randomizer,&mat, false, true, f)) {

			std::cout << "Did succeed.\n";
			std::cout << "A:\n";
			ballot_tools().print_ranked_ballots(f.before_A.election);
			std::cout << "A':\n";
			ballot_tools().print_ranked_ballots(f.after_A.election);
		} else {
			std::cout << "\r" << i << " " << flush;
		}
	}
	std::cout << std::flush;

/*	std::vector<std::vector<test_results> > results_all(4,
		std::vector<test_results>(4));

	for(int scen_a = 0; scen_a < 4; ++scen_a) {
		for (int scen_b = scen_a + 1; scen_b < 4; ++scen_b) {
			results_all[scen_a][scen_b] = test_many(32767,
				canonical_full_v[scen_a], canonical_full_v[scen_b],
				canonical_full_v[scen_a],
				numcands, four_equivalences, other_equivs,
				prospective_functions,randomizer);

			std::cout << std::endl;

		}
	}*/

	test_results testing = test_many(3276,//7,
		/*canonical_full_v[1], canonical_full_v[3],
		copeland_scenario(54, 4),*/
		copeland_scenario(54, 4), copeland_scenario(54, 4), 1,
		canonical_full_v[2],
		numcands, four_equivalences, other_equivs,
		prospective_functions,randomizer);

	std::vector<size_t> test_numcands = {3, 3, 4, 4};

	print_passing_mixed_tuples(testing, prospective_functions,
		test_numcands);

	// There are really 10 tests. 6 tests that involve two scenarios,
	// and 4 tests that involve three. We currently only test the 6.
	// When we'll extend into connecting 3-cand and 4-cand together,
	// there will be more still.

	// print_passing_tuples(results_all, prospective_functions[4]);

	return 0;
}
