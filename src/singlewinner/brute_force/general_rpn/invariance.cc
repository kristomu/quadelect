// Remove algorithms that fail one of two scale invariance tests (relative
// criteria). The two criteria are

// - Multiplying every ballot by a positive constant shouldn't change the
//		outcome.
// - Adding a constant to every ballot shouldn't change the outcome.

// As these are relative tests, they should eventually be folded into the
// test_generator/etc class system. But since they don't care about what
// scenario we're in, it's not very easy to do so at the moment. So this
// is a quick hack that should be improved later.

#include <map>
#include <iterator>
#include <unordered_map>		// Test performance? Needs hash
#include "gen_custom_function.h"

#include <stdlib.h>
#include <fstream>

#include "../../../random/random.h"
#include "../../../spookyhash/SpookyV2.h"

std::vector<double> get_test_vector(int numcands, rng & randomizer) {

	std::vector<double> out(factorial(numcands));
	for (int i = 0; i < factorial(numcands); ++i) {
		out[i] = randomizer.next_double();
	}
	return out;
}

//////////////////////////////////////////////////////////////////////

// Test some absolute criteria
// Maybe todo later, steal some stuff from same_polytope_compositor so
// I don't have to reinvent stride etc every time I make a test routine.

// The stuff below can definitely be optimized.

// Mulitiplicative invariance:
//	f(A) >= f(B) ==> f(Ax) >= f(Bx) for all x in (0..infty)
// I.e. only the relative share of the voters voting any particular
// preference should matter, not how many there are in total.

// Additive invariance:
//	f(A) >= f(B) ==> f(A+x) >= f(B+x) for all x in (0..infty)
// I.e. adding voters who vote for every preference equally should not
// matter.

std::vector<double> added_A, added_B, scaled_A, scaled_B;

bool invariant(rng & randomizer, const gen_custom_function & algorithm) {

	size_t numcands = algorithm.get_num_candidates();

	std::vector<double> test_vector_A = get_test_vector(numcands,
			randomizer);
	std::vector<double> test_vector_B = get_test_vector(numcands,
			randomizer);

	size_t tvlen = test_vector_A.size();

	double tvA_result = algorithm.evaluate(test_vector_A),
		   tvB_result = algorithm.evaluate(test_vector_B);
	// Swap test vector A and B around so that the tvA result is always
	// lesser.
	if (tvA_result > tvB_result) {
		std::swap(test_vector_A, test_vector_B);
		std::swap(tvA_result, tvB_result);
	}

	if (isnan(tvA_result) || isnan(tvB_result)) {
		return false;
	}

	double epsilon = 1e-9;

	std::vector<double> test_factors = {1/128.0, 1/16.0, 16, 128, 1024};

	// Now we must check that the outcome for A is never strictly greater
	// than B.

	scaled_A.resize(tvlen);
	scaled_B.resize(tvlen);

	for (double factor: test_factors) {

		for (size_t i = 0; i < tvlen; ++i) {
			scaled_A[i] = test_vector_A[i] * factor;
			scaled_B[i] = test_vector_B[i] * factor;
		}

		double scaled_A_result = algorithm.evaluate(scaled_A),
			   scaled_B_result = algorithm.evaluate(scaled_B);

		if (isnan(scaled_A_result) || isnan(scaled_B_result)) {
			return false;
		}

		if (scaled_A_result > scaled_B_result + epsilon) {
			return false;
		}
	}

	std::vector<double> test_addends = {2, 4, 8, 9, 128, 1024};

	added_A.resize(tvlen);
	added_B.resize(tvlen);

	for (double addend: test_addends) {

		for (size_t i = 0; i < tvlen; ++i) {
			added_A[i] = test_vector_A[i] + addend;
			added_B[i] = test_vector_B[i] + addend;
		}

		double added_A_result = algorithm.evaluate(added_A),
			   added_B_result = algorithm.evaluate(added_B);

		if (isnan(added_A_result) || isnan(added_B_result)) {
			return false;
		}

		if (added_A_result > added_B_result + epsilon) {
			return false;
		}
	}

	return true;
}

bool invariant(int numiters, rng & randomizer,
	const gen_custom_function & algorithm) {

	for (int i = 0; i < numiters; ++i) {
		if (!invariant(randomizer, algorithm)) {
			return false;
		}
	}

	return true;
}

// Two methods, A and B, give equal results if, for all elections,
// A's score interval for that election overlaps with B's.
// But how to test that in a scalable manner?

// We must consider upper and lower separately! Consider what must be
// the case for the intersection of the score intervals to be empty:
// A's score interval must be to the left of B's or vice versa, i.e.
// A.upper <= B.lower || A.lower >= B.upper. If neither of these are
// true, then we're overlapping.
// But we still need to make that into a test that only uses ordinal
// comparisons of the various A results, and triggers if equal (so we
// can preserve through hashing). Alternatively make something that has
// no false negatives but a bunch of false positives, and then check
// manually.

// I think the easiest approach is to make two cardinal collision maps as
// a no-false-negatives screening mechanism. We assume every interval is of
// length less than x (this can usually be set rather high). Then the first
// map contains round(score(A) * 2x)/2x, and the second map contains
// round((score(A) + x/2)*2x)/2x. If there's an overlap between A and B
// on the interval for the kth election score, then either the first
// or the second rounded value for the kth election is equal for A and B.
// The problem is that which one is equal may well depend on k!

// We have to do it the other way. Start with the first election, and
// sort all methods according to the score. Start at the uppermost (with
// the least score). Every method that's within epsilon of its score
// is considered to potentially be a dupe. (Or overlaps, if it's interval.)
// Anything else is kept in the group of non-dupes.

// More formally: we want to group the algorithms into "same as A",
// "same as B", and so on. Start with every method in the "not a dupe"
// group. Let eps be some epsilon (not needed if we use intervals).
// Generate an election and sort the resulting scores. Then, going from the
// least value to the most:

// Call the algorithm with the current value for x. If x is marked evaluated
// for this round, skip to the next method.
// Let S be the group x is in.
// Separate out, into a new group, all algorithms with results within an
// epsilon of x's, unless that would separate out every algorithm in S.
// Then mark all algorithms thus separated out (trivially always including
// x) as evaluated. Loop to start.

// Run this as many times as you'd please, then each group contains dupes.
// Pick the smallest value algorithm from each group as the sifted output.

int main(int argc, const char ** argv)  {
	int numcands = 4;
	rng randomizer(RNG_ENTROPY);

	// Open some files to get back up to speed

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [file containing past output]"
			<< std::endl;
		return (-1);
	}

	std::string filename = argv[1];
	std::ifstream past_file(filename);

	if (!past_file) {
		std::cerr << "Could not open " << filename << std::endl;
	}

	std::vector<algo_t> prior_algorithm_numbers;
	get_first_token_on_lines(past_file, prior_algorithm_numbers);

	past_file.close();

	// End of our little file diversion

	gen_custom_function x(numcands);

	for (algo_t algorithm: prior_algorithm_numbers) {
		if (!x.set_algorithm(algorithm)) {
			std::cerr << "Error! " << algorithm << " is not a valid algorithm number!"
				<< std::endl;
			throw new std::runtime_error("Error reintroducing algorithms");
		}
		if (!invariant(12000, randomizer, x)) {
			std::cout << "FAIL: " << algorithm << "\t" << x.to_string() << "\n";
		} else {
			std::cout << "pass: " << algorithm << "\t" << x.to_string() << "\n";
		}
	}
}