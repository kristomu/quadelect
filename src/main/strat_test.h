#pragma once

#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <string>

#include <memory>
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
// hash function for elections would fix that problem, but I don't know
// if it's worth it, performance wise.
class test_cache {
	public:
		std::vector<ballots_by_support> grouped_by_challenger;
};

// The strategy (test) classes are ultimately meant to be designed like
// this:
//	They take an index from 0 to a maximum exclusive, and produce some
//	kind of test outcome. If the maximum is -1, then that means there
//  are so many that exhaustive traversal is intractable (wouldn't fit
//  an int64_t).
//  Then there's a method to produce a strategy by index (if the max
//  isn't -1) and one to produce a random strategy.

// Ultimately I want enough machinery to support going through a random
// permutation of these, as well as going through them exhaustively, and
// in such a way that the class doing the iteration (currently
// strategy_test) doesn't have to care about whether the strategy is
// cloning (candidate-centered) or burial/compromise or something else
// entirely... because I want to extend the strategy concept to tests in
// general.

// But I'll have to see if it's doable.

class strategy {
	public:
		// WARNING: EXTRAORDINARILY UGLY HACK. FIX LATER TODO XXX!
		mutable size_t chosen_challenger;

		virtual std::list<ballot_group> get_strategic_election(
			const ordering & honest_outcome,
			int64_t instance_index, const test_cache & cache,
			size_t numcands, pure_ballot_generator * ballot_generator,
			rng * randomizer) const = 0;

		virtual std::string name() const = 0;

		virtual int64_t get_num_tries(size_t numcands) const = 0;
};

class per_ballot_strat : public strategy {
	public:
		virtual ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const = 0;

		std::list<ballot_group> get_strategic_election(
			const ordering & honest_outcome,
			int64_t instance_index, const test_cache & cache,
			size_t numcands, pure_ballot_generator * ballot_generator,
			rng * randomizer) const;

		int64_t get_num_tries(size_t numcands) const {
			// One attempt per challenger
			return numcands-1;
		}
};

class burial : public per_ballot_strat {
	public:
		ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const;

		std::string name() const {
			return "Burial";
		}
};

class compromising : public per_ballot_strat {
	public:
		ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const;

		std::string name() const {
			return "Compromising";
		}
};

class two_sided_strat : public per_ballot_strat {
	public:
		ballot_group modify_ballots(ballot_group ballot,
			size_t winner, size_t challenger) const;

		std::string name() const {
			return "Two-sided";
		}
};

// Reverse the honest outcome and place the challenger first and
// the winner last.
// E.g. if the social outcome was W=A>B>C>D=E, and the winner is W and
// challenger C, the strategic faction all vote C>D=E>B>A>W.
class two_sided_reverse : public strategy {
	public:
		std::list<ballot_group> get_strategic_election(
			const ordering & honest_outcome,
			int64_t instance_index, const test_cache & cache,
			size_t numcands, pure_ballot_generator * ballot_generator,
			rng * randomizer) const;

		std::string name() const {
			return "Two-sided reverse";
		}

		int64_t get_num_tries(size_t numcands) const {
			// One attempt per challenger
			return numcands-1;
		}

};

class coalitional_strategy : public strategy {

	public:
		std::list<ballot_group> get_strategic_election(
			const ordering & honest_outcome,
			int64_t instance_index, const test_cache & cache,
			size_t numcands, pure_ballot_generator * ballot_generator,
			rng * randomizer) const;

		std::string name() const {
			return "Coalitional strategy";
		}

		int64_t get_num_tries(size_t numcands) const {
			// Too many to count.
			return -1;
		}
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

		// This should be pushed into a list or vector or somesuch,
		// but we first need to somehow signal that coalitional strategy
		// can be tried lots of times but the others can only be tried
		// once. And deal with the hack that is set_num_candidates().
		// TODO.
		std::vector<std::unique_ptr<strategy> > strategies;
		coalitional_strategy coalstrat;

		strategy * get_applicable_strategy(int iteration);

		// TODO: Call this something else so that we don't get
		// "strategy" (manipulation) confused with "strategies"
		// (burial, compromise, etc.)
		strategy_result attempt_execute_strategy();

		bool too_many_ties;
		size_t ballot_gen_idx;

		ballots_by_support group_by_support(
			const std::list<ballot_group> & ballots,
			size_t winner, size_t challenger) const;

	public:
		// Returns true if the strategy succeeded, false
		// otherwise. TODO: Should return a disproof instead.
		// ... if desired?

		bool strategize_for_election(
			const std::list<ballot_group> & ballots,
			ordering honest_outcome, size_t numcands, bool verbose);

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

			strategies.push_back(std::make_unique<burial>());
			strategies.push_back(std::make_unique<compromising>());
			strategies.push_back(std::make_unique<two_sided_strat>());
			strategies.push_back(std::make_unique<two_sided_reverse>());
			strategies.push_back(std::make_unique<coalitional_strategy>());
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