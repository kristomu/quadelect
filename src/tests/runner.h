#pragma once

#include "../bandit/tests/tests.h"
#include "../tools/tools.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"

#include "../generator/ballotgen.h"
#include "../singlewinner/method.h"
#include "../random/random.h"

#include "test.h"

#include <memory>

enum strategy_result { STRAT_FAILED, STRAT_SUCCESS, STRAT_TIE };

// TODO: Rename classes! "Test" here is a multi-armed bandit thing...

class test_runner : public Test {
	private:
		rng * randomizer;
		int numvoters;
		size_t numcands_min, numcands_max;
		int total_generation_attempts;
		election_method * method;
		std::list<ballot_group> ballots;
		pure_ballot_generator * ballot_gen;
		pure_ballot_generator * strat_generator;
		size_t strategy_attempts_per_try;

		std::vector<std::shared_ptr<criterion_test> > tests;
		std::vector<bool> failed_criterion;
		bool last_run_tried_all_tests;

		// TODO: Call this something else as we're now about
		// tests, not strategies.
		strategy_result attempt_finding_failure(bool only_one);

		bool too_many_ties;
		size_t ballot_gen_idx;

		std::string runner_name;

	public:
		// Returns the number of failed criteria. If only_one is true,
		// it exits as soon as some failure was found. This is used for
		// fast "any-of" testing when we only need to know that something
		// failed, not which.
		// Side effect: also populates the failed_criterion array.
		// (TODO: Something less ugly.)
		size_t get_num_failed_criteria(
			const std::list<ballot_group> & ballots,
			ordering honest_outcome, size_t numcands,
			bool only_one, bool verbose);

		// This gives a map indexed by test name, so that the ith
		// boolean value is true if we could find a failure of that
		// test for the ith election, false otherwise.
		// NOTE: This returns the results for the last exhaustive test.
		// If none has been done, it throws an exception.
		std::map<std::string, bool> get_failure_pattern() const;

		// This just executes a test first.
		std::map<std::string, bool> calculate_failure_pattern() {
			attempt_finding_failure(false);
			return get_failure_pattern();
		}

		void add_test(const std::shared_ptr<criterion_test> & in) {
			tests.push_back(in);
			failed_criterion.push_back(false);
		}

		test_runner(pure_ballot_generator * ballot_gen_in,
			pure_ballot_generator * strat_generator_in,
			int numvoters_in, int numcands_in_min, int numcands_in_max,
			rng & randomizer_in, election_method * method_in, int index,
			int strat_attempts_per_try_in) {

			total_generation_attempts = 0;
			last_run_tried_all_tests = false;

			assert(numvoters_in > 0);
			assert(numcands_in_min > 0);
			assert(numcands_in_max >= 0);
			assert(strat_attempts_per_try_in > 0);

			randomizer = &randomizer_in;
			method = method_in;
			numvoters = numvoters_in;
			numcands_min = numcands_in_min;
			numcands_max = numcands_in_max;
			ballot_gen = ballot_gen_in;
			strat_generator = strat_generator_in;
			too_many_ties = false;
			ballot_gen_idx = 0;

			strategy_attempts_per_try = strat_attempts_per_try_in;
			runner_name = "Criterion test"; // default name
		}

		void set_name(std::string new_name) {
			runner_name = new_name;
		}

		test_runner(pure_ballot_generator * ballot_gen_in,
			pure_ballot_generator * strat_generator_in,
			int numvoters_in, int numcands_in,
			rng & randomizer_in, election_method * method_in, int index,
			int strat_attempts_per_try_in) : test_runner(ballot_gen_in,
					strat_generator_in, numvoters_in, numcands_in, numcands_in,
					randomizer_in, method_in, index, strat_attempts_per_try_in) {}

		double perform_test() {
			// We need to make a theoretically coherent system for handling
			// ties. Ties should hurt more the more often they appear.

			switch (attempt_finding_failure(true)) {
				case STRAT_SUCCESS:
				case STRAT_TIE:
					return (0);
				case STRAT_FAILED:
					return (1);
				default:
					throw std::invalid_argument(
						"unknown strategy type");
			}
		}

		std::string name() const {
			return (runner_name + " (" +
					method->name() + ")");
		}

		int get_total_generation_attempts() const {
			return (total_generation_attempts);
		}

		double get_minimum() const {
			return (0);
		}
		double get_maximum() const {
			return (1);
		}
};