#pragma once

#include <vector>
#include <string>

// Storage class for results from testing algorithms/functions against
// test instances. This is used because we can't keep all the data in
// memory when the number of functions grow very large (e.g. 100M).

// It also supports in-memory storage in case I find myself on a computer
// with a TB of RAM.

typedef float test_t;

enum test_election { TYPE_A = 0, TYPE_B = 1, 
	TYPE_A_PRIME = 2, TYPE_B_PRIME = 3 };

const int NUM_REL_ELECTION_TYPES = 4;

class test_results {
	private:
		void set_size_variables();

		size_t get_linear_idx(size_t method_idx, 
			size_t test_instance_number, test_election type) const;

		void allocate_space_memory() {
			// File stuff goes here later.
			results = new test_t[total_num_entries];
			allocated = true;
		}

		void allocate_space_disk(std::string filename);
		void finish_space_disk();

		int fd;

	public:
		// This is really a 3D array: [test number][method idx][A,B,A',B']
		// but we use a 1D array because it's going to be saved to disk.
		test_t * results;
		bool allocated;

		// TODO: Hash the list of methods for A, B, A', B', so that we
		// can verify that we're using the right list of methods -- since
		// results is indexed by method index (kth method on the list),
		// using the wrong list will produce utter garbage.

		// start_of_type is the start of all data that has to do with
		// a particular 

		std::vector<size_t> num_methods, start_of_type;
		size_t total_num_entries;
		size_t num_tests;

		// Assume the same number of methods for each test election type.
		test_results(int num_tests_in, int num_methods_in) {
			num_tests = num_tests_in;
			num_methods = std::vector<size_t>(4, num_methods_in);
			set_size_variables();
			allocated = false;
		}

		~test_results() {
			if (allocated) {
				finish_space_disk();
				//delete results;
			}
		}

		void allocate_space(std::string fn) { allocate_space_disk(fn); }

		size_t get_bytes_required() const {
			return sizeof(test_t) * total_num_entries;
		}

		void set_result(int method_idx, int test_instance_number,
			test_election type, test_t result) {
			results[get_linear_idx(method_idx, test_instance_number,
				type)] = result;
		}

		test_t get_result(int method_idx, int test_instance_number,
			test_election type) const {
			return results[get_linear_idx(method_idx, 
				test_instance_number, type)];
		}

		// Check if a given combination of methods pass all the
		// tests according to the recorded results.

		bool passes_tests(const std::vector<int> method_indices) const;
};