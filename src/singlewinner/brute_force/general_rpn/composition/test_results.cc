#include "test_results.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

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

	// Check that the indices make sense.

	if (type > NUM_REL_ELECTION_TYPES) {
		throw std::runtime_error(
			"get_linear_idx: unknown type");
	}

	if (method_idx >= num_methods[type]) {
		throw std::runtime_error(
			"get_linear_idx: method idx > num methods");
	}

	if (test_instance_number >= num_tests) {
		throw std::runtime_error(
			"get_linear_idx: test_instance_number > num tests");
	}

	// Find the linear index into the results array.
	// The digit system is like this
	// [0..4: type] [0...num_methods: method number] 
	//					[0...num_test_instances : test number]
	return test_instance_number + method_idx * num_tests +
		start_of_type[type];
}

// https://stackoverflow.com/questions/29210851

void test_results::allocate_space_disk(std::string filename) {
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
}

void test_results::finish_space_disk() {
	 if (munmap((void *)results, get_bytes_required()) < 0) {
		throw std::runtime_error("test_results: Could not unmap file");
	}
}

bool test_results::passes_tests(
	const std::vector<int> method_indices, bool no_harm,
	bool no_help) const {

	// The method fails a no-harm test if A has greater score than B in the
	// before scenario, but has lower score than B in the after scenario,
	// because going from before to after is supposed to always help A
	// more than it helps some non-A candidate.

	// Conversely, it fails no-help if A has lower score than B in the
	// before scenario but higher after. 

	// Caching will also happen later if required.
	// (I thought I had also done this!)

	std::vector<std::vector<test_t> > cur_combo_results(
		NUM_REL_ELECTION_TYPES);

	for (int type = 0; type < NUM_REL_ELECTION_TYPES; ++type) {
		size_t start = get_linear_idx(method_indices[type], 0, 
			(test_election)type);
		std::copy(results + start, results + start + num_tests, 
			std::back_inserter(cur_combo_results[type]));
	}

	// There are three possibilities: either the test is both no-harm and
	// no-help, it's only no-harm, or it's only no-help.
	// The fourth option (neither no-harm nor no-help) trivially returns
	// true all the time.

	bool before_direction, after_direction;

	for (size_t i = 0; i < num_tests; ++i) {
		before_direction = cur_combo_results[TYPE_A][i] - 
			cur_combo_results[TYPE_B][i] > 0;
		after_direction = cur_combo_results[TYPE_A_PRIME][i] - 
			cur_combo_results[TYPE_B_PRIME][i] > 0;

		if (no_harm) {
			// If the signs are different, someone who was losing is now
			// winning or vice versa, so return false.
			if (no_help && (before_direction ^ after_direction)) {
				return false;
			}
			// If A went from winner to loser over B, return false.
			if (before_direction && !after_direction) {
				return false;
			}
		} else {
			if (no_help && (!before_direction && after_direction)) {
				return false;
			}
		}
	}

	return true;
}