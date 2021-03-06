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

// TODO?: Use actual OO random number generators so we can control them for
// reproducibility.

enum strategy_result { STRAT_FAILED, STRAT_SUCCESS, STRAT_TIE };

class StrategyTest : public Test {
	private:
		rng * randomizer;
		int numvoters;
		size_t numcands;
		int total_generation_attempts;
		election_method * method;
		list<ballot_group> ballots;
		vector<pure_ballot_generator *> ballot_gens;
		pure_ballot_generator * strat_generator;
		int strategy_attempts_per_try;

		strategy_result attempt_execute_strategy(); // Mega method, fix later

		bool too_many_ties;
		size_t ballot_gen_idx;

	public:

		StrategyTest(vector<pure_ballot_generator *> ballot_gens_in,
			pure_ballot_generator * strat_generator_in,
			int numvoters_in, int numcands_in, rng & randomizer_in,
			election_method * method_in, int index,
			int strat_attempts_per_try_in) {

			total_generation_attempts = 0;

			assert (numvoters_in > 0);
			assert (numcands_in > 0);
			assert (strat_attempts_per_try_in > 0);

			randomizer = &randomizer_in;
			method = method_in;
			numvoters = numvoters_in;
			numcands = numcands_in;
			ballot_gens = ballot_gens_in;
			strat_generator = strat_generator_in;
			too_many_ties = false;
			ballot_gen_idx = 0;

			// Was 384
			strategy_attempts_per_try = strat_attempts_per_try_in;
		}

		double perform_test() {
			// We need to make a theoretically coherent system for handling
			// ties. Ties should hurt more the more often they appear.
			// (Also should find out how the cancellation method really
			// worked to see if anything can be salvaged from it.)

			switch(attempt_execute_strategy()) {
				case STRAT_SUCCESS:
				case STRAT_TIE:
					return(0);
				case STRAT_FAILED:
					return(1);
				default:
					throw invalid_argument(
						"unknown strategy type");
			}
		}
		std::string name() const { return ("Strategy(multiple/" +
			method->name() + ")");}

		int get_total_generation_attempts() const {
			return(total_generation_attempts); }

		double get_minimum() const { return(0); }
		double get_maximum() const { return(1); }
};

#endif
