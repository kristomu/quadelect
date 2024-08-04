#pragma once

#include <vector>
#include "../scenario.h"
#include "../test_instance_gen.h"
#include "../logistics/vector_test_instance.h"

#include "../test_results.h"

// A test generator group is a group of test instance generators that
// output elections that make use of the same scenarios, and that are
// to be tested in the same way, and thus can be tested on the same tuple of algorithms.

// The group also contains a sample method that returns a number of such
// elections by sampling from each generator in turn, so that generating
// tests for a particular scenario tuple (over all available tests relevant
// to that tuple) is made easy.

// Also makes use of the reversal trick (see .cc) to include more test
// instance generators in the same group.

class test_generator_group {
	private:
		std::vector<test_instance_generator> generators;
		std::vector<bool> should_be_reversed;
		copeland_scenario before_A, after_A, before_B, after_B;

		bool no_harm, no_help;

		vector_test_instance sample(test_instance_generator & generator,
			bool reverse, const std::map<int, fixed_cand_equivalences> &
			candidate_equivalences) const;

		bool fits_group_directly(
			const test_instance_generator & candidate) const;

		bool fits_group_reversed(
			const test_instance_generator & candidate) const;

	public:
		std::vector<vector_test_instance> sample(
			size_t desired_samples,
			const std::map<int, fixed_cand_equivalences> &
			candidate_equivalences);

		bool fits_group(const test_instance_generator & candidate) const;

		bool get_no_harm() const {
			return no_harm;
		}
		bool get_no_help() const {
			return no_help;
		}

		void insert(test_instance_generator candidate);

		void print_members() const;
		void print_scenarios(std::ostream & where) const;

		std::set<copeland_scenario> get_tested_scenarios() const;

		// used for producing an ordering among groups with the same
		// scenarios.
		bool operator<(const test_generator_group & other) const;
		bool operator==(const test_generator_group & other) const;

		copeland_scenario get_scenario(test_election election_type) const;
};
