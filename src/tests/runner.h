#pragma once

#include "../simulator/bernoulli.h"
#include "../tools/tools.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"

#include "../generator/ballotgen.h"
#include "../singlewinner/method.h"
#include "../random/random.h"

#include "test.h"

#include <memory>
#include <stdexcept>

enum test_result { TEST_NO_DISPROOFS, TEST_DISPROVEN, TEST_TIE };

class test_runner : public bernoulli_simulator {
	private:
		int numvoters;
		size_t numcands_min, numcands_max;
		int total_generation_attempts;
		std::shared_ptr<const election_method> method;
		election_t ballots;
		std::shared_ptr<pure_ballot_generator> ballot_gen;
		size_t disproof_attempts_per_election;

		std::vector<std::shared_ptr<criterion_test> > tests;
		std::set<std::string> admitted_test_names;

		std::vector<bool> failed_criteria;
		bool last_run_tried_all_tests;

		// TODO: Call this something else as we're now about
		// tests, not strategies.
		test_result attempt_finding_failure(bool only_one);

		size_t ballot_gen_idx;

		std::string runner_name;

		double find_scale_factor() {
			return 1; // no scaling needed
		}

	public:
		// Returns the number of failed criteria. If only_one is true,
		// it exits as soon as some failure was found. This is used for
		// fast "any-of" testing when we only need to know that something
		// failed, not which.
		// Side effect: also populates the failed_criteria array.
		// (TODO: Something less ugly.)
		size_t get_num_failed_criteria(
			const election_t & ballots,
			ordering honest_outcome, size_t numcands,
			bool only_one, bool verbose);

		// This gives a map indexed by test name, so that the ith
		// boolean value is true if we could find a failure of that
		// test for the ith election, false otherwise.
		// NOTE: This returns the results for the last exhaustive test.
		// If nothing has been done, it throws an exception; this
		// currently includes the case where the honest election
		// produces a tie. (See calculate_failure_pattern below.)
		std::map<std::string, bool> get_failure_pattern() const;

		// This just executes a test first. It returns an empty map if
		// a tie happened - this is rather ugly, but since ties can't
		// be considered exceptional, I can't throw an exception...
		// perhaps use std::optional or somesuch? TODO?
		std::map<std::string, bool> calculate_failure_pattern() {
			test_result res = attempt_finding_failure(false);
			if (res == TEST_TIE) {
				return {};
			}
			return get_failure_pattern();
		}

		void add_test(const std::shared_ptr<criterion_test> & in) {

			if (admitted_test_names.find(in->name()) !=
				admitted_test_names.end()) {
				throw std::invalid_argument(
					"test_runner: Tried to add the same test more than once.");
			}

			tests.push_back(in);
			admitted_test_names.insert(in->name());
			failed_criteria.push_back(false);
		}

		test_runner(std::shared_ptr<pure_ballot_generator> ballot_gen_in,
			int numvoters_in, int numcands_in_min, int numcands_in_max,
			std::shared_ptr<coordinate_gen> randomizer_in,
			std::shared_ptr<const election_method> method_in,
			int attempts_per_election_in) : bernoulli_simulator(randomizer_in) {

			total_generation_attempts = 0;
			last_run_tried_all_tests = false;

			if (numvoters_in < 0 || numcands_in_min < 0
				|| numcands_in_max < numcands_in_min
				|| attempts_per_election_in < 0) {

				throw std::out_of_range("test_runner: input parameter "
					"negative or out of range");
			}

			method = method_in;
			numvoters = numvoters_in;
			numcands_min = numcands_in_min;
			numcands_max = numcands_in_max;
			ballot_gen = ballot_gen_in;
			ballot_gen_idx = 0;

			disproof_attempts_per_election = attempts_per_election_in;
			runner_name = "Criterion test"; // default name
		}

		void set_name(std::string new_name) {
			runner_name = new_name;
		}

		test_runner(std::shared_ptr<pure_ballot_generator> ballot_gen_in,
			int numvoters_in, int numcands_in,
			std::shared_ptr<coordinate_gen> randomizer_in,
			std::shared_ptr<const election_method> method_in,
			int strat_attempts_per_try_in) : test_runner(ballot_gen_in,
					numvoters_in, numcands_in, numcands_in, randomizer_in,
					method_in, strat_attempts_per_try_in) {}

		// This samples from the Bernoulli distribution of failure rates: it
		// returns 0 if we couldn't find a failure, or 1 if one was found.
		// The convention for strategy resistance values is to return the
		// manipulability, not the resistance, thus we do it like this.

		// NOTE: This might lead to a problem later on if/when we generalize
		// this class to return various criterion failures, because my
		// monotonicity tester uses the other convention (returning success
		// rates, not failure rates). One has to give if we're to be consistent.

		double do_simulation() {
			// TODO: We need to make a theoretically coherent system for handling
			// ties. Ties should hurt more the more often they appear. For now,
			// ties are considered to be strategy failures (i.e. instances of
			// manipulability). Perhaps later allow these to return an exceptional
			// value "the simulation failed to produce a result".

			switch (attempt_finding_failure(true)) {
				case TEST_DISPROVEN:
				case TEST_TIE:
					return 1;
				case TEST_NO_DISPROOFS:
					return 0;
				default:
					// Should be an assert?
					throw std::invalid_argument(
						"test_runner::simulate - unknown return code");
			}
		}

		std::string name() const {
			return runner_name + " (" +
				method->name() + ")";
		}

		int get_total_generation_attempts() const {
			return total_generation_attempts;
		}

		bool higher_is_better() const {
			return false;
		};
};