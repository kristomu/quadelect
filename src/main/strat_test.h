#ifndef __TEST_STRATEGY
#define __TEST_STRATEGY

#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <string>

#include <stdexcept>

#include "../bandit/tests/tests.h"
#include "../tools/tools.h"

#include "../tools/ballot_tools.h"
#include "../ballots.h"

#include "../generator/ballotgen.h"
#include "../singlewinner/method.h"
#include "../random/random.h"

enum strategy_result { STRAT_FAILED, STRAT_SUCCESS, STRAT_TIE };

enum basic_strategy {ST_NONE = -1, ST_BURIAL = 0, ST_COMPROMISING = 1,
	ST_TWOSIDED = 2, ST_REVERSE = 3, ST_OTHER = 4
};

class StrategyTest : public Test {
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

		strategy_result attempt_execute_strategy(); // Mega method, fix later

		bool too_many_ties;
		size_t ballot_gen_idx;

	public:

		StrategyTest(pure_ballot_generator * ballot_gen_in,
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

			// Was 384
			strategy_attempts_per_try = strat_attempts_per_try_in;
		}

		StrategyTest(pure_ballot_generator * ballot_gen_in,
			pure_ballot_generator * strat_generator_in,
			int numvoters_in, int numcands_in,
			rng & randomizer_in, election_method * method_in, int index,
			int strat_attempts_per_try_in) : StrategyTest(ballot_gen_in,
					strat_generator_in, numvoters_in, numcands_in, numcands_in,
					randomizer_in, method_in, index, strat_attempts_per_try_in) {}

		// Used mostly for debugging, maybe clean up later. This returns
		// true if it's possible to make someone besides the winner win.
		// XXX: UB may happen if there's a tie.
		basic_strategy strategize_for_election(const std::list<ballot_group> &
			ballots,
			ordering honest_outcome, size_t numcands,
			bool verbose) const;

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
			return ("Strategy(multiple/" +
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

#endif
