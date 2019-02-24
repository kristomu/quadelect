#include "test_results.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdexcept>

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

	if (method_idx >= num_methods[type]) {
		throw std::runtime_error(
			"get_linear_idx: method idx > num methods");
	}

	if (test_instance_number >= num_tests) {
		throw std::runtime_error(
			"get_linear_idx: method idx > num methods");	
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
	const std::vector<int> method_indices) const {

	// The method fails a test if A has greater score than B in the
	// before scenario, but has lower score than B in the after scenario,
	// because going from before to after is supposed to always help A
	// more than it helps some non-A candidate.

	// (Note: cloning is more strict than this: handle later when we
	// start dealing with cloning.)

	// Caching will also happen later if required.

	std::vector<std::vector<test_t> > cur_combo_results(
		NUM_REL_ELECTION_TYPES);

	for (int type = 0; type < NUM_REL_ELECTION_TYPES; ++type) {
		size_t start = get_linear_idx(method_indices[type], 0, 
			(test_election)type);
		std::copy(results + start, results + start + num_tests, 
			std::back_inserter(cur_combo_results[type]));
	}

	for (size_t i = 0; i < num_tests; ++i) {
		if (cur_combo_results[TYPE_A][i] - cur_combo_results[TYPE_B][i] > 0 
			&& cur_combo_results[TYPE_A_PRIME][i] -
			   cur_combo_results[TYPE_B_PRIME][i] < 0) {
			return false;
		}
	}

	return true;
}