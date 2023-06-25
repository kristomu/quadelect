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
		int strategy_attempts_per_try;

		std::vector<std::shared_ptr<criterion_test> > strategies;

		// TODO: Call this something else so that we don't get
		// "strategy" (manipulation) confused with "strategies"
		// (burial, compromise, etc.)
		strategy_result attempt_execute_strategy();

		bool too_many_ties;
		size_t ballot_gen_idx;

		std::string runner_name;

	public:
		// Returns true if the strategy succeeded, false
		// otherwise. TODO: Should return a disproof instead.
		// ... if desired?

		bool strategize_for_election(
			const std::list<ballot_group> & ballots,
			ordering honest_outcome, size_t numcands, bool verbose);

		void add_test(const std::shared_ptr<criterion_test> & in) {
			strategies.push_back(in);
		}

		test_runner(pure_ballot_generator * ballot_gen_in,
			pure_ballot_generator * strat_generator_in,
			int numvoters_in, int numcands_in_min, int numcands_in_max,
			rng & randomizer_in, election_method * method_in, int index,
			int strat_attempts_per_try_in) {

			total_generation_attempts = 0;

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

			switch (attempt_execute_strategy()) {
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