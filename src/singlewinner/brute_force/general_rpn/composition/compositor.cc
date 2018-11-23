
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
	int from_perspective_of;
};

// Throws exception if permuting to that scenario is impossible.
// If consider_A is false, we only look at permutations that map some
// other candidate to A (this is useful for monotonicity).

// TODO: Investigate why we can't remap derived scenarios to nonderived
// ones.
election_scenario_pair permute_to_desired(election_scenario_pair cur,
	const copeland_scenario & desired_scenario, bool consider_A,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
		candidate_remappings) {

	if (cur.scenario == desired_scenario) {
		return cur;
	}

	size_t numcands = cur.scenario.get_numcands();

	for (size_t i = 0; i < numcands; ++i) {
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

		return cur;
	}

	throw std::runtime_error(
		"permute_to_desired: could not remap " + cur.scenario.to_string() +
		" to " + desired_scenario.to_string());
}

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

// This is somewhat ugly and repeated code. Fix later.
// To handle reversed situations properly (see below for what that is),
// we need to know the proper scenarios for A, A', B, and B'.
copeland_scenario get_B_prime_scenario(
	const copeland_scenario & A_scenario,
	const copeland_scenario & B_scenario,
	const copeland_scenario & A_prime_scenario,
	const std::vector<std::map<copeland_scenario, isomorphism> > &
		candidate_remappings) {

	// Make a phantom election_scenario_pair corresponding to A.
	election_scenario_pair A;
	A.scenario = A_scenario;
	A.from_perspective_of = 0;

	// This gives us the candidate that corresponds to the B scenario.
	election_scenario_pair B = permute_to_desired(A, B_scenario,
		false, candidate_remappings);

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

// Return a list of all scenarios that can be turned into desired_scenario
// by permuting the candidates. Since "can be turned into another scenario"
// is an equivalence relation on scenarios, determining this is as easy as
// determining what scenarios we can turn desired_scenario into by permuting
// the candidates (symmetry property).

std::set<copeland_scenario> get_permitted_scenarios(
	const copeland_scenario & desired_scenario, size_t numcands,
	const fixed_cand_equivalences & equivalences) {

	std::set<copeland_scenario> out;

	for (size_t i = 0; i < numcands; ++i) {
		out.insert(equivalences.get_candidate_remapping(
			desired_scenario, i).to_scenario);
		/*if (candidate_remappings[i].find(desired_scenario) ==
			candidate_remappings[i].end()) {
			throw std::runtime_error(
				"get_permitted_scenarios: could not find source scenario!");
		}

		out.insert(candidate_remappings[i].find(desired_scenario)->
			second.to_scenario);*/

	}

	return out;
}

// BEWARE: depends on some assumptions about smith set size 4 behavior!
// (Namely that you can go from any Smith set size 4 scenario to any other
// by permuting the candidates.)
// Fix later: proper solution finds equivalence classes of what scenarios
// can be reached from others by candidate permutations.
// If it finds anything, returns true and sets out. Otherwise returns false,
// and the contents of out are undefined.

// Permitted_scenarios: scenarios that can be turned into
// desired_before_scenario and desired_after_scenario.

// TODO: permitted_scenarios is a vector or map into the equivalence classes
// for each scenario, i.e. permitted_scenarios[55,4] = everything that can
// be turned into 55,4. The problem is that if we do a vector, we lose
// numcand independence (i.e. we'd implement the int typecast for scneario
// to return 55 in the case above), and if we do map, the lookup cost is
// pretty large, and the while(permitted_scenarios.find()) bit is pretty
// slow. We could hack it by looking up the required set in the map only
// once (before the do), though.

// An idea: Instead of changing the elections, only keep the permutation,
// and combine different permutations.
// That could work better when we start to introduce ISDA.

// Hotspot
bool get_test_instance(const copeland_scenario * desired_A_scenario,
	const copeland_scenario * desired_B_scenario,
	const copeland_scenario * desired_Aprime_scenario,
	const std::set<copeland_scenario> & permitted_scenarios,
	const fixed_cand_equivalences & equivalences, int numcands,
	rng & randomizer, monotonicity * mono_test, bool reverse,
	monotonicity_test_instance & out) {

	impartial ic(false);

	election_scenario_pair before, after;

	int num_ballots;

	// Get the B prime scenario.
	copeland_scenario B_prime = get_B_prime_scenario(
		*desired_A_scenario, *desired_B_scenario, *desired_Aprime_scenario,
		equivalences.get_cand_remaps());

	const copeland_scenario * desired_Bprime_scenario = &B_prime;

	// If we're reversed, we want to first generate something with
	// desired_Bprime_scenario for the A election (and desired_Aprime_scenario
	// for the B election), and then swap them around using
	// reverse_transform. So in that case what the user specified as
	// desired_A_transform should internally be desired_Bprime_transform and
	// vice versa.
	if (reverse) {
		std::swap(desired_A_scenario, desired_Bprime_scenario);
		std::swap(desired_B_scenario, desired_Aprime_scenario);
	}

	// Generate an acceptable ballot.

	do {
		// Use an odd number of voters to avoid ties.
		num_ballots = 2 * randomizer.lrand(5, 50) + 1;
		before.election = ic.generate_ballots(num_ballots, numcands,
			randomizer);
		before.scenario = copeland_scenario(before.election, numcands);
	} while (permitted_scenarios.find(before.scenario) ==
		permitted_scenarios.end());

	// Permute into the A_scenario we want.
	before = permute_to_desired(before, *desired_A_scenario, true,
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
		int num_to_add = randomizer.lrand(1, 5);
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
		false, equivalences.get_cand_remaps());

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

	assert(out.before_A.from_perspective_of ==
		out.after_A.from_perspective_of);
	assert(out.after_B.from_perspective_of ==
		out.before_B.from_perspective_of);

	if (reverse) {
		out = reverse_transform(out);
	}

	return true;
}

typedef float result_t;

struct test_results {
	copeland_scenario A_scenario, B_scenario;
	std::vector<std::vector<result_t> > A_before_results;
	std::vector<std::vector<result_t> > A_after_results;
	std::vector<std::vector<result_t> > B_before_results;
	std::vector<std::vector<result_t> > B_after_results;
};

struct

test_results test_many(int numiters,
	const copeland_scenario & desired_A_scenario,
	const copeland_scenario & desired_B_scenario,
	int numcands, const fixed_cand_equivalences & equivalences,
	const std::vector<algo_t> & functions_to_test, rng & randomizer) {

	struct timespec last_time, next_time;

	clock_gettime(CLOCK_MONOTONIC, &last_time);

	test_results results;
	results.A_scenario = desired_A_scenario;
	results.B_scenario = desired_B_scenario;

	size_t num_functions = functions_to_test.size();

	// Ew. Fix later.
	results.A_before_results.resize(num_functions);
	results.A_after_results.resize(num_functions);
	results.B_before_results.resize(num_functions);
	results.B_after_results.resize(num_functions);

	std::set<copeland_scenario> permitted = get_permitted_scenarios(
		desired_A_scenario, numcands, equivalences);

	monotonicity_test_instance test_instance;

	mono_add_top mattest(false, false);
	mono_raise mrtest(false, false);
	monotonicity * mono_test;

	gen_custom_function evaluator(numcands);

	int time_count = 0;
	int time_to_check = 1000;

	for (int i = 0; i < numiters; ++i) {

		// Do we want mono-raise or mono-add-top? Decide with a coin flip.
		if (randomizer.drand() < 0.5) {
			mono_test = &mrtest;
		} else {
			mono_test = &mattest;
		}

		// Do we want a reverse test? Decide with a coin flip!

		bool reverse = randomizer.drand() < 0.5;

		// Run get_instance until we get a result that fits what we want.
		while (!get_test_instance(&desired_A_scenario, &desired_B_scenario,
			&desired_A_scenario, permitted,
			equivalences, numcands, randomizer, mono_test, reverse,
			test_instance));

		assert(test_instance.before_A.scenario == desired_A_scenario);
		assert(test_instance.before_B.scenario == desired_B_scenario);

		// Prepare ballot vectors.
		std::vector<std::vector<double> > ballot_vectors = {
			get_ballot_vector(test_instance.before_A.election, numcands),
			get_ballot_vector(test_instance.before_B.election, numcands),
			get_ballot_vector(test_instance.after_A.election, numcands),
			get_ballot_vector(test_instance.after_B.election, numcands)
		};

		// Run test against everything.
		for (size_t j = 0; j < num_functions; ++j) {

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
		}
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

int main(int argc, char ** argv) {

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	int numcands = 4;

	// Open some files to get back up to speed

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [file containing sifter output]"
			<< std::endl;
		return(-1);
	}

	std::string filename = argv[1];
	std::ifstream sifter_file(filename);

	if (!sifter_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	std::vector<algo_t> prospective_functions;
	get_first_token_on_lines(sifter_file, prospective_functions);

	sifter_file.close();

	fixed_cand_equivalences four_equivalences(4);


/*	std::map<copeland_scenario, isomorphism> scenario_reductions =
		get_derived_scenario_reductions(numcands);

	std::vector<std::map<copeland_scenario, isomorphism> > cand_remaps =
		get_candidate_remappings(numcands, scenario_reductions);*/

	std::set<copeland_scenario> nonderived_full = get_nonderived_scenarios(4,
		four_equivalences);

	std::vector<copeland_scenario> nonderived_full_v;
	std::copy(nonderived_full.begin(), nonderived_full.end(),
		std::back_inserter(nonderived_full_v));

	copeland_scenario example_desired_A, example_desired_B;

	// I don't know why references don't work here...
	for (copeland_scenario x: nonderived_full) {
		std::cout << "Smith set 4 nonderived: " << x.to_string() << std::endl;
	}

	example_desired_A = nonderived_full_v[0];
	example_desired_B = nonderived_full_v[1];

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
	std::set<copeland_scenario> permitted = get_permitted_scenarios(
		nonderived_full_v[0], numcands, four_equivalences);

	monotonicity_test_instance f;

	mono_add_top mat(false, false);

	std::cout << "Before get_instance" << std::endl;

	for (int i = 0; i < 100; ++i){

		copeland_scenario three_cand_smith_one(54, 4),
			three_cand_smith_two(5, 4);

		/*if (get_test_instance(&nonderived_full_v[0], &nonderived_full_v[1],
			&nonderived_full_v[2], &nonderived_full_v[1], permitted, cand_remaps,
			4, randomizer, &mat, false, f))*/
		if (get_test_instance(&nonderived_full_v[0], &nonderived_full_v[1],
			&three_cand_smith_one, permitted, four_equivalences, 4,
			randomizer,&mat, false, f)) {

			std::cout << "Did succeed.\n";
			std::cout << "A:\n";
			ballot_tools().print_ranked_ballots(f.before_A.election);
			std::cout << "A':\n";
			ballot_tools().print_ranked_ballots(f.after_A.election);
		} else {
			std::cout << ".";
		}
	}
	std::cout << std::flush;

	/*std::copy(prospective_functions.begin(), prospective_functions.end(),
		ostream_iterator<algo_t>(std::cout, "\n"));*/

	std::vector<std::vector<test_results> > results_all(4,
		std::vector<test_results>(4));

	for(int scen_a = 0; scen_a < 4; ++scen_a) {
		for (int scen_b = scen_a + 1; scen_b < 4; ++scen_b) {
			results_all[scen_a][scen_b] = test_many(32767,
				nonderived_full_v[scen_a], nonderived_full_v[scen_b],
				numcands, four_equivalences, prospective_functions,randomizer);

			std::cout << std::endl;

			/*print_passing_pairs(results, prospective_functions); */
		}
	}

	// There are really 10 tests. 6 tests that involve two scenarios,
	// and 4 tests that involve three. We currently only test the 6.
	// When we'll extend into connecting 3-cand and 4-cand together,
	// there will be more still.

	print_passing_tuples(results_all, prospective_functions);

	return 0;
}
