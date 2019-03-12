// Program for testing lots of gen_custom_function algorithms and removing the
// redundant ones (those that respond the same exact way as ones we've
// already seen).

// This is done by testing each algorithm with a number of (randomly chosen)
// vectors, and then comparing the results to what's already stored. There
// are two such tests: cardinal ones (where we directly compare the results
// to what we've already seen) and ordinal ones (where we compare the order
// of the results, i.e. which test returned max score, which returned next to
// max and so on, to the orders we've already seen).

#include <map>
#include <unordered_map>		// Test performance? Needs hash
#include "gen_custom_function.h"

#include <stdlib.h>
#include <fstream>

#include "../../../spookyhash/SpookyV2.h"

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

std::vector<std::vector<double> > get_identity_vectors(int numcands,
	bool scale_by_pos) {

	std::vector<std::vector<double> > out;
	std::vector<double> zeroes(factorial(numcands), 0), identity;
	identity = zeroes;
	for (int i = 0; i < factorial(numcands); ++i) {
		if (scale_by_pos) {
			identity[i] = i/10.0;
		} else {
			identity[i] = 1;
		}
		out.push_back(identity);
		identity[i] = 0;
	}
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

	for (size_t x = 0; x < cardinal_results.size(); ++x) {
		for (size_t y = 0; y < cardinal_results.size(); ++y) {
			if (x == y) continue;

			ordinal_output.push_back((cardinal_results[x]-cardinal_results[y])
				> threshold);
		}
	}

	return ordinal_output;
}

// I make up for the laziness by just running everything through a good hash.
// The result is that being too generous with the bits to represent the order
// of the test result numbers no longer matters, as everything gets squished
// down to 128 bits anyway. And as long as we have fewer than 2^64 different
// values, we should be at negligible risk of a collision.

typedef std::pair<uint64_t, uint64_t> hash_result;

namespace std
{
	template<>
	struct hash<std::pair<uint64_t, uint64_t> >
	{
		// Assumes the key numbers are well-distributed, as with the
		// results of the spooky hash above.
		size_t operator()(const std::pair<uint64_t, uint64_t> & in) const {
			return (size_t)in.first;
		}
	};
}

hash_result get_ordinal_hash_test_result(const std::vector<double> &
	cardinal_results, SpookyHash & hasher) {

	char ordinal_output[square(cardinal_results.size())];

	// If two values are within an epsilon of threshold, consider them
	// to be equal.
	double threshold = 1e-9;

	int ctr = 0;

	for (size_t x = 0; x < cardinal_results.size(); ++x) {
		for (size_t y = 0; y < cardinal_results.size(); ++y) {
			if ((cardinal_results[x]-cardinal_results[y]) > threshold) {
				ordinal_output[ctr++] = '0';
			} else {
				ordinal_output[ctr++] = '1';
			}
		}
	}

	hash_result output;
	hasher.Hash128(ordinal_output, square(cardinal_results.size()),
		&output.first, &output.second);

	return output;
}

// The only 4-cddt algorithm test I can think of that won't require us to
// choose multiple of them at once to handle the Smith set rotation -- is
// the invariance to multiplying every vote value by a constant. That can
// be tested just by creating two test sets: one that's a constant multiple
// of the other - and then compare the ordinal test results for both
// (adjusting epsilon as necessary).

std::string algo_str(int numcands, algo_t algorithm) {
	return gen_custom_function(numcands, algorithm).to_string();
}

int main(int argc, const char ** argv)  {
	SpookyHash hasher;
	hasher.Init(0, 0);

	int numcands = 4;

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

	gen_custom_function x(numcands);

	std::vector<std::vector<double> > cardinal_tests = get_test_vectors(8,
		numcands);

	std::vector<std::vector<double> > ordinal_tests = get_test_vectors(48,
		numcands);

	std::vector<std::vector<double> > basis = get_identity_vectors(numcands,
		true);

	std::copy(basis.begin(), basis.end(), std::back_inserter(ordinal_tests));

	std::map<std::vector<double>, algo_t> cardinal_seen_before;
	std::unordered_map<std::pair<uint64_t, uint64_t>, algo_t> ordinal_seen_before;

	// Insert all the algorithms we've already been through...
	algo_t last_algorithm_seen = 0;
	int count = 0;

	for (algo_t algorithm: prior_algorithm_numbers) {
		if (!x.set_algorithm(algorithm)) {
			std::cerr << "Error! " << algorithm << " is not a valid algorithm number!" << std::endl;
			throw new std::runtime_error("Error reintroducing algorithms");
		}

		last_algorithm_seen = std::max(last_algorithm_seen, algorithm);

		// Assume it's a new algorithm - don't bother checking with
		// ordinal_seen_before, just introduce it right away.

		std::pair<uint64_t, uint64_t> ordinal_results_hash =
			get_ordinal_hash_test_result(
				evaluate_algorithm(x, ordinal_tests), hasher);

		ordinal_seen_before[ordinal_results_hash] = algorithm;

		if ((++count & 2047) == 0) {
			std::cerr << algorithm/(long double)last_algorithm_seen << "    \r" << std::flush;
			count = 0;
		}
	}

	prior_algorithm_numbers.clear();

	// And now for some new ones.

	for (algo_t i = last_algorithm_seen; ; ++i) {
		if (!x.set_algorithm(i)) continue;

		std::vector<double> cardinal_results = evaluate_algorithm(x,
			cardinal_tests);

		if (cardinal_seen_before.find(cardinal_results) !=
			cardinal_seen_before.end()) {

			continue;
		}

		std::pair<uint64_t, uint64_t> ordinal_results_hash =
			get_ordinal_hash_test_result(
				evaluate_algorithm(x, ordinal_tests), hasher);

		if (ordinal_seen_before.find(ordinal_results_hash) !=
			ordinal_seen_before.end()) {
			continue;
		}

		std::cout << i << "\t" << x.to_string() << std::endl;
		ordinal_seen_before[ordinal_results_hash] = i;
	}
}