#pragma once

#include "scenario.h"
#include "test_generator.h"

// Test tuple generator. Each such corresponds to a single
// (A,A',B,B', candidate number for B, relative criterion) tuple. By
// creating every possible combination of these tuples, we can cover
// all the test sets for an election method constructed from
// gen_custom_functions, for a given set of relative criteria that
// we want the method to pass. The division of the whole search space
// into these tuples also makes divide and conquer possible.

class test_instance_generator {
	public:
		copeland_scenario before_A, after_A, before_B, after_B;
		int cand_B_idx;
		test_generator tgen;

		test_instance_generator(test_generator in) : tgen(in) {}

		relative_test_instance get_test_instance(
			const std::map<int, fixed_cand_equivalences> 
			candidate_equivalences);
};