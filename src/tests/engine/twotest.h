// Testing engine for relative criteria (two_tests). A relative criterion
// is a constraint on the outcomes of two ballot sets that are similar in
// some way.

// This testing engine improves upon our previous by being cache-friendly.
// It is currently only single-winner.

// See the functions for description of how the cache-friendly approach works.

// Perhaps support tie failures and take actual ties into account.
// Tie failures would be something like (for monotonicity), first A = B = C > D,
// then we find failures for A > ..., B > ..., C > ..., and so no matter how
// the method would break the tie, it can't escape.

#ifndef _VOTE_TWOTEST_TTE
#define _VOTE_TWOTEST_TTE

#include "../../generator/ballotgen.h"
#include "../two_tests.h"

#include <iostream>
#include <vector>
#include <map>

using namespace std;

class twotest_engine {

	private:
		// Tests, methods, and generator in use.
		vector<twotest *> tests;
		vector<const election_method *> methods;
		pure_ballot_generator * generator;

		// Map from name to vector index
		map<string, int> test_index;
		map<string, int> method_index;

		// Data about which methods fail which criteria, and if so,
		// disproofs of criterion compliance. This is test-major, i.e.
		// data for method x and criterion y is [y][x].
		// TODO BLUESKY: Some kind of way we can specify beforehand
		// criteria we do know the method fails.
		vector<vector<method_test_info> > method_pass_status;

		// TODO: Distinguish between internal iterations and external
		// ones.
		unsigned int num_iterations;

		// Maximum and minimum number of voters and candidates to test.
		int max_num_voters, max_num_candidates;
		int min_num_voters, min_num_candidates;

	public:
		twotest_engine(unsigned int max_iters, int min_voters,
			int max_voters, int min_cands, int max_cands);
		twotest_engine();

		void set_generator(pure_ballot_generator * gen_in);
		void set_maxiters(unsigned int max_iters);
		void set_num_voters(int minimum, int maximum);
		void set_num_candidates(int minimum, int maximum);

		// Setters and getters later.
		// TODO: Some way of adding methods, duh!

		// TIRED FIX LATER. :p
		// Should be made const, perhaps - particularly e_m.
		void add_method(const election_method * method_in);
		// Twotest can't be made const since they store disproofs
		// internally.
		void add_test(twotest * twotest_in);

		// Returns false if there's nothing to test, otherwise true.
		bool run_tests(int iterations, rng & random_source);
};

#endif
