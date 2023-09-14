
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
		if (i == 0 && !consider_A) {
			continue;
		}

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

		assert(cur.from_perspective_of == desired_candidate
			|| desired_candidate == -1);

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

	if (desired_candidate == 0) {
		return cur;
	}

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
	if (reduced_numcands == in.scenario.get_numcands()) {
		return in;
	}

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

impartial ic(false);

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
		std::pair<bool, std::list<ballot_group> > alteration = mono_test->
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

void test_many_one_scenario(int numiters, size_t & global_iter_count,
	std::vector<bool> & passes_so_far,
	const copeland_scenario & desired_scenario, int desired_B_candidate,
	int numcands, const fixed_cand_equivalences & equivalences,
	const std::map<int, fixed_cand_equivalences> & other_equivalences,
	const std::vector<algo_t> & functions_to_test, rng & randomizer) {

	monotonicity_test_instance test_instance;

	mono_add_top mattest(false, false);
	mono_raise mrtest(false, false);
	monotonicity * mono_test;

	size_t passes;
	gen_custom_function evaluator(numcands);

	for (int iter = 0; iter < numiters; ++iter) {

		// Do we want mono-raise or mono-add-top? Decide with a coin flip.
		if (randomizer.next_double() < 0.5) {
			mono_test = &mrtest;
		} else {
			mono_test = &mattest;
		}

		while (!get_test_instance(&desired_scenario, &desired_scenario,
				desired_B_candidate, &desired_scenario, equivalences,
				other_equivalences, numcands, randomizer, mono_test, false,
				true, test_instance));

		std::vector<std::vector<double> > ballot_vectors = {
			get_ballot_vector(test_instance.before_A),
			get_ballot_vector(test_instance.before_B),
			get_ballot_vector(test_instance.after_A),
			get_ballot_vector(test_instance.after_B)
		};

		// Quick and dirty scaling to catch methods that only fail
		// on non-integer outputs.
		double scale_factor = randomizer.next_double(0.5, 1);
		// Reduce numerical imprecision by making the floating point
		// integral divided by a power of two.
		scale_factor = round(scale_factor * 65536) / 65536.0;

		for (size_t j = 0; j < ballot_vectors.size(); ++j) {
			for (size_t k = 0; k < ballot_vectors[j].size(); ++k) {
				ballot_vectors[j][k] *= scale_factor;
			}
		}

		passes = 0;

		for (size_t funct_idx = 0; funct_idx < functions_to_test.size();
			++funct_idx) {

			// If we already know this method is unsuitable, skip.
			if (!passes_so_far[funct_idx]) {
				continue;
			}

			algo_t to_test = functions_to_test[funct_idx];

			// If it's the first iteration, check that the algorithm is
			// valid. If it's not the first iteration, then all the
			// algorithms must have been valdi, so skip the verification
			// step in set_algorithm, as that's costly.

			if (global_iter_count == 0) {
				assert(evaluator.set_algorithm(to_test));
			} else {
				evaluator.force_set_algorithm(to_test);
			}

			double result_A = evaluator.evaluate(ballot_vectors[0]),
				   result_B = evaluator.evaluate(ballot_vectors[1]),
				   result_Ap = evaluator.evaluate(ballot_vectors[2]),
				   result_Bp = evaluator.evaluate(ballot_vectors[3]);

			if (result_A - result_B > 0 && result_Ap - result_Bp < 0) {
				passes_so_far[funct_idx] = false;
				std::cout << "Disqualified " << to_test << " at iteration " <<
					global_iter_count << "\n";
			} else {
				++passes;
			}
		}
		++global_iter_count;
	}
	std::cout << "Passes: " << passes << std::endl;
}


int main(int argc, char ** argv) {

	int seed = 114;
	rng randomizer(seed);

	srand(seed);
	srandom(seed);
	srand48(seed);

	// Integrity test.

	gen_custom_function integrity_test(3);
	if (!integrity_test.test()) {
		throw std::runtime_error("Compositor: failed integrity test!");
	}

	int numcands = 3;

	// Open some files to get back up to speed

	std::cout << "Reading file... " << std::endl;

	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] <<
			" [file containing sifter output, 3 cands] [output file]" <<
			std::endl;
		return (-1);
	}

	std::string filename = argv[1];
	std::ifstream sifter_file(filename);

	if (!sifter_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	std::vector<std::vector<algo_t> > prospective_functions(5);

	get_first_token_on_lines(sifter_file, prospective_functions[3]);

	sifter_file.close();

	std::cout << "... done (read " << prospective_functions.size()
		<<  " functions)." << std::endl;

	std::cout << "Initializing equivalences..." << std::endl;

	std::string out_filename = argv[2];

	fixed_cand_equivalences three_equivalences(3);

	std::map<int, fixed_cand_equivalences> other_equivs;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(4,
			fixed_cand_equivalences(4)));;
	other_equivs.insert(std::pair<int, fixed_cand_equivalences>(3,
			fixed_cand_equivalences(3)));;

	std::set<copeland_scenario> canonical_full = get_nonderived_scenarios(3,
			three_equivalences);

	std::vector<copeland_scenario> canonical_full_v;
	std::copy(canonical_full.begin(), canonical_full.end(),
		std::back_inserter(canonical_full_v));

	std::cout << "... done." << std::endl;

	copeland_scenario example_desired_A, example_desired_B;

	// I don't know why references don't work here...
	for (copeland_scenario x: canonical_full) {
		std::cout << "Smith set 3 canonical: " << x.to_string() << std::endl;
	}

	std::cout << "Initializing pass array..." << std::endl;

	std::vector<bool> passes_so_far(prospective_functions[3].size(), true);

	std::cout << "... done." << std::endl;

	size_t global_iter_count = 0;
	gen_custom_function evaluator(numcands);

	int who = 1;

	for (;;) {
		if (who > 2) {
			who = 1;
		}

		std::cout << "From the perspective of " << who << std::endl;

		test_many_one_scenario(200, global_iter_count, passes_so_far,
			canonical_full_v[0], who++,
			numcands, three_equivalences, other_equivs, prospective_functions[3],
			randomizer);

		std::ofstream out_file(out_filename);

		for (size_t i = 0; i < prospective_functions[3].size(); ++i) {
			if (passes_so_far[i]) {
				evaluator.force_set_algorithm(prospective_functions[3][i]);
				out_file << prospective_functions[3][i] << "\t"
					<< evaluator.to_string() << "\n";
			}
		}

		out_file.close();

	}

	return 0;
}
