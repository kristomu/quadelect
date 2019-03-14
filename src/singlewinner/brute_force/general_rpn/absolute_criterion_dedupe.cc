// Remove algorithms that fail a scale invariance test or that are too
// alike other algorithms (thus deduping algorithm sets that seem to be
// different but are only so due to floating point errors).

// This all uses interval arithmetic. There are some known bugs, and
// the second step hasn't been implemented yet.

#include <map>
#include <unordered_map>		// Test performance? Needs hash
#include "gen_custom_function.h"

#include <stdlib.h>
#include <fstream>

#include <boost/numeric/interval.hpp>

#include "../../../interval/interval.h"
#include "../../../random/random.h"
#include "../../../spookyhash/SpookyV2.h"

template<typename T> std::vector<T> get_test_vector(int numcands, 
	rng & randomizer) {
	
	std::vector<T> out(factorial(numcands));
	for (int i = 0; i < factorial(numcands); ++i) {
		out[i] = T(randomizer.drand());
	}
	return out;
}

//////////////////////////////////////////////////////////////////////

// Test some absolute criteria
// Maybe todo later, steal some stuff from same_polytope_compositor so
// I don't have to reinvent stride etc every time I make a test routine.

// The stuff below can definitely be optimized.

// f(A) >= f(B) ==> f(Ax) >= f(Bx) for all x in (0..infty)
// I.e. only the relative share of the voters voting any particular
// preference should matter, not how many there are in total.
bool multiplicatively_invariant(rng & randomizer,
	const gen_custom_function<Interval> & algorithm) {

	int numcands = 4; // HACK, FIX

	std::vector<Interval> test_vector_A = get_test_vector<Interval>(
		numcands, randomizer);
	std::vector<Interval> test_vector_B = get_test_vector<Interval>(
		numcands, randomizer);

	Interval tvA_result = algorithm.evaluate(test_vector_A),
			 tvB_result = algorithm.evaluate(test_vector_B);
	// Swap test vector A and B around so that tvA is to the left of
	// tvB.
	if (boost::numeric::lower(tvA_result) > boost::numeric::lower(tvB_result)) {
		std::swap(test_vector_A, test_vector_B);
		std::swap(tvA_result, tvB_result);
	}

	//std::cout << boost::numeric::lower(tvA_result) << ", " << boost::numeric::upper(tvA_result) << std::endl;

	std::vector<double> test_factors = {0.01, 0.1, 10, 100, 1000};

	// Now we must check that the interval A is never strictly greater 
	// than B, i.e. disjoint with A's lower end greater than B's upper.
	// If so, we have a failure.

	for (double factor: test_factors) {
		std::vector<Interval> scaled_A, scaled_B;

		for (Interval pref_A : test_vector_A) {
			scaled_A.push_back(pref_A * factor);
		}

		for (Interval pref_B : test_vector_B) {
			scaled_B.push_back(pref_B * factor);
		}

		Interval scaled_A_result = algorithm.evaluate(scaled_A),
			scaled_B_result = algorithm.evaluate(scaled_B);

		if (boost::numeric::lower(scaled_A_result) > boost::numeric::upper(scaled_B_result)) {
			return false;
		}
	}
	return true;
}

bool multiplicatively_invariant(int numiters, rng & randomizer,
	const gen_custom_function<Interval> & algorithm) {

	for (int i = 0; i < numiters; ++i) {
		if (!multiplicatively_invariant(randomizer, algorithm)) {
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
/*int main() {
	int numcands = 4;

	gen_custom_function<Interval> x(numcands);
	x.set_algorithm(249218781332);

	std::vector<Interval> in;
	for (int i = 0; i < 24; ++i) {
		in.push_back(i);
	}

	Interval f = x.evaluate(in);

	std::cout << boost::numeric::lower(f) << ", " << boost::numeric::upper(f) << std::endl;
	std::cout << boost::numeric::upper(f) - boost::numeric::lower(f) << std::endl;
}*/

int main(int argc, const char ** argv)  {
	int numcands = 4;
	rng randomizer(RNG_ENTROPY);

	// Open some files to get back up to speed

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [file containing past output]"
			<< std::endl;
		return(-1);
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

	gen_custom_function<Interval> x(numcands);

	for (algo_t algorithm: prior_algorithm_numbers) {
		if (!x.set_algorithm(algorithm)) {
			std::cerr << "Error! " << algorithm << " is not a valid algorithm number!" << std::endl;
			throw new std::runtime_error("Error reintroducing algorithms");
		}
		if (!multiplicatively_invariant(1000, randomizer, x)) {
			std::cout << "FAIL: " << x.to_string() << std::endl;
		} else {
			std::cout << "pass: " << x.to_string() << std::endl;
		}
	}
}