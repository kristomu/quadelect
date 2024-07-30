#include "test_results.h"
#include "../../../../tools/tools.h"

// HACK: MSVC doesn't support mmap. There's an alternate implementation
// at https://github.com/alitrack/mman-win32, which should be investigated
// later, but for now just throw an exception.

// The reason I throw an exception instead of just removing this tool
// from the MSVC CMakeLists is that I want to check that the rest
// compiles.

#if !defined(_MSC_VER)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <assert.h>

#include <stdexcept>
#include <iostream>

void test_results::set_size_variables() {

	size_t start_of_cur_type = 0, end_of_cur_type = 0;

	// Find the linear index into the results array.
	// The addressing system for the results array is like this, from
	// most significant digit to the least.
	// [0..4: type] [0...num_methods: method number]
	//					[0...num_test_instances : test number]

	// Since num_methods may differ based on type, we have to
	// calculate the size of each to find out where different types
	// start.

	start_of_type.resize(NUM_REL_ELECTION_TYPES);

	for (int type = 0; type < NUM_REL_ELECTION_TYPES; ++type) {
		start_of_cur_type = end_of_cur_type; // from previous type
		end_of_cur_type = start_of_cur_type + num_methods[type] *
			num_tests,
			start_of_type[type] = start_of_cur_type;
	}

	total_num_entries = end_of_cur_type;
}

size_t test_results::get_linear_idx(size_t method_idx,
	size_t test_instance_number, test_election type) const {

	// If debugging, check that the indices make sense.
	// The following checks are assertions so that when not
	// debugging, we don't slow down the program too much.

	assert((int)type < NUM_REL_ELECTION_TYPES);
	assert(method_idx < num_methods[type]);
	assert(test_instance_number < num_tests);

	// Find the linear index into the results array.
	// The digit system is like this
	// [0..4: type] [0...num_methods: method number]
	//					[0...num_test_instances : test number]
	return test_instance_number + method_idx * num_tests +
		start_of_type[type];
}

// https://stackoverflow.com/questions/29210851

void test_results::allocate_space_disk(std::string filename) {
#if defined(_MSC_VER)
	throw std::runtime_error("Saving to disk not implemented on Windows yet");
#else
	fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IREAD | S_IWRITE);

	if (fd < 0) {
		throw std::runtime_error(
			"test_results: Could not open file for memory mapping");
	}

	if (fallocate(fd, 0, 0, get_bytes_required()) < 0) {
		throw std::runtime_error("test_results: Could not resize file");
	}

	void * mem_map = mmap(NULL, get_bytes_required(),
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (mem_map == MAP_FAILED) {
		throw std::runtime_error(
			"test_results: Could not memory-map file");
	}

	results = (test_t *)mem_map;
#endif
}

void test_results::finish_space_disk() {
#if defined(_MSC_VER)
	throw std::runtime_error("Saving to disk not implemented on Windows yet");
#else
	if (munmap((void *)results, get_bytes_required()) < 0) {
		throw std::runtime_error("test_results: Could not unmap file");
	}
#endif
}

bool test_results::passes_tests(
	const std::vector<int> & method_indices, bool no_harm,
	bool no_help) const {

	// The method fails a no-harm test if A has greater score than B in the
	// before scenario, but has lower score than B in the after scenario,
	// because going from before to after is supposed to always help A
	// more than it helps some non-A candidate.

	// Conversely, it fails no-help if A has lower score than B in the
	// before scenario but higher after.

	// First determine the location of the first test result for each
	// algorithm.

	for (int type = 0; type < NUM_REL_ELECTION_TYPES; ++type) {
		results_start_position[type] = get_linear_idx(method_indices[type],
				0, (test_election)type);
	}

	// There are three possibilities: either the test is both no-harm and
	// no-help, it's only no-harm, or it's only no-help.
	// The fourth option (neither no-harm nor no-help) trivially returns
	// true all the time.

	for (size_t i = 0; i < num_tests; ++i) {
		test_t result_A  = results[results_start_position[TYPE_A]+i],
			   result_B  = results[results_start_position[TYPE_B]+i],
			   result_Ap = results[results_start_position[TYPE_A_PRIME]+i],
			   result_Bp = results[results_start_position[TYPE_B_PRIME]+i];

		int before_sign = sign(result_A - result_B),
			after_sign = sign(result_Ap - result_Bp);

		// If B was ranked higher than A before, but lower after, then A was
		// helped. If B was ranked lower then A before, but higher after,
		// then A was harmed.

		// These also take into account (used to lose, but now is tied) and
		// (used to be tied, but now is winning) transitions.

		bool helped = before_sign < after_sign,
			 harmed = before_sign > after_sign;

		if (no_harm && harmed) {
			return false;
		}
		if (no_help && helped) {
			return false;
		}
	}

	return true;
}

// Increments the counter vector for all indices that correspond to a
// failed test.
bool test_results::passes_tests(
	const std::vector<int> & method_indices,
	std::vector<size_t> & fail_counts, size_t must_pass_first_k,
	bool no_harm, bool no_help) const {

	assert(fail_counts.size() == num_tests);

	bool fail = false;

	for (int type = 0; type < NUM_REL_ELECTION_TYPES; ++type) {
		results_start_position[type] = get_linear_idx(method_indices[type],
				0, (test_election)type);
	}

	for (size_t i = 0; i < num_tests; ++i) {
		test_t result_A  = results[results_start_position[TYPE_A]+i],
			   result_B  = results[results_start_position[TYPE_B]+i],
			   result_Ap = results[results_start_position[TYPE_A_PRIME]+i],
			   result_Bp = results[results_start_position[TYPE_B_PRIME]+i];

		int before_sign = sign(result_A - result_B),
			after_sign = sign(result_Ap - result_Bp);

		bool helped = before_sign < after_sign,
			 harmed = before_sign > after_sign;

		if ((no_harm && harmed) || (no_help && helped)) {
			// If it's one of the first k tests (that we must pass), abort
			// outright.
			if (i < must_pass_first_k) {
				return false;
			}
			// Otherwise mark this test as failing.
			++fail_counts[i];
			fail = true;
		}
	}

	return fail;
}

// Swap all test results at test index first_test with those at
// index second_test.
void test_results::swap(size_t first_test, size_t second_test) {
	if (first_test == second_test) {
		return;
	}
	assert(first_test < num_tests && second_test < num_tests);

	// [0..4: type] [0...num_methods: method number]
	//					[0...num_test_instances : test number]

	for (size_t type = 0; type < NUM_REL_ELECTION_TYPES; ++type) {
		for (size_t method = 0; method < num_methods[type]; ++method) {
			size_t first_loc = get_linear_idx(method,
					first_test, (test_election)type);
			size_t second_loc = get_linear_idx(method,
					second_test, (test_election)type);
			assert(first_loc < total_num_entries);
			assert(second_loc < total_num_entries);
			// BEWARE! Using std::swap() without std:: in front causes a
			// segfault!
			std::swap(results[first_loc], results[second_loc]);
		}
	}
}
