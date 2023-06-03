#pragma once

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

// Refactoring scaffolding. Since we're trying to engineer
// a strategy in favor of the challenger, we divide the ballots
// into those that explicitly support the challenger and everybody
// else. Those that are indifferent to the winner and challenger
// thus don't count as supporting the challenger.
class ballots_by_support {
	public:
		std::list<ballot_group> supporting_challenger,
			others;

		double challenger_support, other_support;
		size_t winner, challenger;

		ballots_by_support(size_t winner_in, size_t challenger_in) {
			challenger_support = 0;
			other_support = 0;
			winner = winner_in;
			challenger = challenger_in;
		}
};


// This is rather ugly: because the system will be trying to create a
// strategy based on a single election a number of times, we can't have
// it reconstruct the data required to figure out a strategy every time;
// it'll just be too slow. So somehow the data has to be preserved between
// strategic attempts, but because this is meant to be a general cache for
// all sorts of tests (monotonicity, etc), it must cut across the otherwise
// modular test architecture. Hence the mess. But doing it otherwise is
// either very slow (calculating it all the time) or even messier (bundling
// tests that use the same processed information together).

// XXX: This doesn't yet contain a hash that will identify the election it's
// valid for. It must thus never be used across elections. Implementing a
// hash function for
class test_cache {
	public:
		std::vector<ballots_by_support> grouped_by_challenger;
};

class strategy {
	public:
		virtual std::list<ballot_group> get_strategic_election(
			const ballots_by_support & grouped_ballots,
			const test_cache & cache, size_t numcands,
			pure_ballot_generator * ballot_generator, rng * randomizer) = 0;

		virtual std::string name() const = 0;
};

class per_ballot_strat : public strategy {
	public:
		virtual ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const = 0;

		std::list<ballot_group> get_strategic_election(
			const ballots_by_support & grouped_ballots,
			const test_cache & cache, size_t numcands,
			pure_ballot_generator * ballot_generator, rng * randomizer);
};

class burial : public per_ballot_strat {

};

class compromising : public per_ballot_strat {

};

class strategy_test : public Test {
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

		strategy_result attempt_execute_strategy();

		bool too_many_ties;
		size_t ballot_gen_idx;

		ballots_by_support group_by_support(
			const std::list<ballot_group> & ballots,
			size_t winner, size_t challenger) const;

		static ballot_group bury(ballot_group ballot, size_t winner,
			size_t challenger);

		static ballot_group compromise(ballot_group ballot, size_t winner,
			size_t challenger);

		static ballot_group two_sided_strat(ballot_group ballot, size_t winner,
			size_t challenger);

		std::list<ballot_group> do_simple_strat(const ballots_by_support &
			grouped_ballots,
			ballot_group(*strat)(ballot_group, size_t,size_t)) const;

		std::list<ballot_group> two_sided_reverse(const ballots_by_support &
			grouped_ballots, const ordering & honest_outcome) const;

		std::list<ballot_group> coalitional_strategy(const ballots_by_support &
			grouped_ballots, size_t numcands, size_t num_coalitions,
			pure_ballot_generator * ballot_generator, rng * randomizer) const;

	public:
		// Returns true if the strategy succeeded, false
		// otherwise. TODO: Should return a disproof instead.

		bool strategize_for_election(
			const std::list<ballot_group> & ballots,
			ordering honest_outcome, size_t numcands, bool verbose) const;

		strategy_test(pure_ballot_generator * ballot_gen_in,
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

		strategy_test(pure_ballot_generator * ballot_gen_in,
			pure_ballot_generator * strat_generator_in,
			int numvoters_in, int numcands_in,
			rng & randomizer_in, election_method * method_in, int index,
			int strat_attempts_per_try_in) : strategy_test(ballot_gen_in,
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