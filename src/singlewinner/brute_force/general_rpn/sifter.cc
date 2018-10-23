// Program for testing lots of gen_custom_function algorithms and removing the
// redundant ones (those that respond the same exact way as ones we've 
// already seen).

// This is done by testing each algorithm with a number of (randomly chosen)
// vectors, and then comparing the results to what's already stored. There
// are two such tests: cardinal ones (where we directly compare the results
// to what we've already seen) and ordinal ones (where we compare the order
// of the results, i.e. which test returned max score, which returned next to
// max and so on, to the orders we've already seen).

// TODO: Use a secure hash to only need 128 bits per unique solution instead
// of a very large amount of memory per.

#include <map>
#include "rpn_evaluator.cc"

#include <stdlib.h>

// Will clean up and put into class later.

std::vector<double> get_test_vector(int numcands) {
	std::vector<double> out(factorial(numcands));
	std::generate(out.begin(), out.end(), []() { return(drand48()*1000); });
	return out;
}

std::vector<std::vector<double> > get_test_vectors(int how_many, 
	int numcands) {

	std::vector<std::vector<double> > out(how_many);
	std::generate(out.begin(), out.end(), [numcands]() { 
		return(get_test_vector(numcands)); });

	return out;
}

std::vector<double> evaluate_algorithm(gen_custom_function & gcf, 
	//algo_t algorithm_number, 
	const std::vector<std::vector<double> > & test_vectors) {

	// If it's not a well-formed algorithm, then abort immediately.
	/*if (!gcf.set_algorithm(algorithm_number)) {
		return std::vector<double>();
	}*/

	// Otherwise, evaluate on all the test vectors.
	std::vector<double> output;
	for (const std::vector<double> & test_vector : test_vectors) {
		output.push_back(gcf.evaluate(test_vector));
	}

	return output;
}

// I'm lazy; instead of turning the cardinal results into a permutation
// and mapping the permutation to an integer (which would be very large
// anyway), I just make an n^2 boolean vector based on whether the ith
// entry is greater than the jth for all (i,j) combinations.
std::vector<bool> get_ordinal_test_result(const std::vector<double> & 
	cardinal_results) {

	std::vector<bool> ordinal_output;

	// If two values are within an epsilon of threshold, consider them
	// to be equal.
	double threshold = 1e-9;

	for (double x: cardinal_results) {
		for (double y: cardinal_results) {
			ordinal_output.push_back(fabs(x-y) > threshold);
		}
	}

	return ordinal_output;
}

// The only 4-cddt algorithm test I can think of that won't require us to
// choose multiple of them at once to handle the Smith set rotation -- is
// the invariance to multiplying every vote value by a constant. That can
// be tested just by creating two test sets: one that's a constant multiple
// of the other - and then compare the ordinal test results for both 
// (adjusting epsilon as necessary).

main() {
	int numcands = 4;

	gen_custom_function x(numcands);

	std::vector<std::vector<double> > cardinal_tests = get_test_vectors(8,
		numcands);

	std::vector<std::vector<double> > ordinal_tests = get_test_vectors(32,
		numcands);

	std::map<std::vector<double>, algo_t> cardinal_seen_before;
	std::map<std::vector<bool>, algo_t> ordinal_seen_before;

	for (algo_t i = 0;; ++i) {
		if (!x.set_algorithm(i)) continue;

		std::vector<double> cardinal_results = evaluate_algorithm(x, //i,
			cardinal_tests);

		if (cardinal_seen_before.find(cardinal_results) != 
			cardinal_seen_before.end()) {
			continue;
		}

		std::vector<bool> ordinal_results = get_ordinal_test_result(
			evaluate_algorithm(x, /*i,*/ ordinal_tests));

		if (ordinal_seen_before.find(ordinal_results) !=
			ordinal_seen_before.end()) {
			//cardinal_seen_before[cardinal_results] = i;
			continue;
		}

		// Idea: if we get here, look up what matches the ordinal results.
		// Get ordinal for both it and us with say, 128 test vectors. If
		// they're equal, then skip. Might be too slow?

		std::cout << i << "\t" << x.to_string() << std::endl;
		ordinal_seen_before[ordinal_results] = i;
	}
}