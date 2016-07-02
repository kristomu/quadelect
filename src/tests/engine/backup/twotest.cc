// Testing engine for relative criteria (two_tests). A relative criterion
// is a constraint on the outcomes of two ballot sets that are similar in
// some way.

// This testing engine improves upon our previous by being cache-friendly.
// It is currently only single-winner.

// Each round goes like this:
// 	- Generate a base ballot set
//	     Group together all tests that can build upon this ballot set,
//	     i.e. returns applicable(outcome, data, true) as true.
//               For each test,
//                  Generate a modified ballot set.
//                   Group together all methods for which this modified ballot
//                   can be used, i.e. returns applicable(mod. outcome, data,
//                   false) as true.
//                     Test these and mark pass/fail.
//                   Repeat until all methods have been tested or we run out of
//                   iterations. Clear mod cache between tries.
//                Next. Clear mod cache between tests.
//      - Clear unmod caches.
//      - Repeat until done.

// Perhaps support tie failures and take actual ties into account.
// Tie failures would be something like (for monotonicity), first A = B = C > D,
// then we find failures for A > ..., B > ..., C > ..., and so no matter how 
// the method would break the tie, it can't escape.

#include "../../generator/ballotgen.cc"
#include "../two_tests.cc"

#include <iostream>
#include <vector>
#include <map>

using namespace std;

class twotest_engine {

	private:
		// Tests, methods, and generator in use.
		vector<twotest *> tests;
		vector<election_method *> methods;
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

		unsigned int num_iterations;

		// Maximum and minimum number of voters and candidates to test.
		int max_num_voters, max_num_candidates;
		int min_num_voters, min_num_candidates;

		// Generate a random int on [minimum...maximum>
		int bracket_random(int minimum, int maximum) const;

	public:
		twotest_engine(unsigned int max_iters, int min_voters,
				int max_voters, int min_cands, int max_cands);

		void set_generator(pure_ballot_generator * gen_in);
		void set_maxiters(unsigned int max_iters, bool effective);
		void set_num_voters(int minimum, int maximum);
		void set_num_candidates(int minimum, int maximum);

		// Setters and getters later.
		// TODO: Some way of adding methods, duh!

		// Returns false if there's nothing to test, otherwise true.
		bool run_tests(int iterations);
};

// Maybe put this in tools, instead.
// Not *truly* equiprobable because random()s maxval isn't a factorial, but eh,
// close enough. One way of doing it right would be to find the greatest 2^n,
// then just throw away numbers that are too large.
int twotest_engine::bracket_random(int minimum, int maximum) const {
	assert (minimum <= maximum);
	
	if (maximum-minimum < 2) return(minimum);

	return(minimum + random() % (maximum - minimum));
}

void twotest_engine::set_generator(pure_ballot_generator * gen_in) {
	generator = gen_in;
}

void twotest_engine::set_generator(pure_ballot_generator * gen_in) {
        generator = gen_in;
}

void twotest_engine::set_maxiters(unsigned int max_iters, bool effective) {
        num_iterations = max_iters;
        eff_iterations = effective;
}

void twotest_engine::set_num_voters(int minimum, int maximum) {
        min_num_voters = minimum;
        max_num_voters = maximum;
}

void twotest_engine::set_num_candidates(int minimum, int maximum) {
        min_num_candidates = minimum;
        max_num_candidates = maximum;
}

twotest_engine::twotest_engine() {
        generator = NULL;
        set_maxiters(1024, false);
        set_num_voters(1, 100);
        set_num_candidates(1, 10);
}

twotest_engine::twotest_engine(unsigned int max_iters, bool effective,
                int min_voters, int max_voters, int min_cands, int max_cands) {
        generator = NULL;
        set_maxiters(max_iters, effective);
        set_num_voters(min_voters, max_voters);
        set_num_candidates(min_cands, max_cands);
}

// Should be vector<ternary> either inapp for inapplicable, or true/false.
// For the test in question, we generate random data and then check which
// methods (method outcomes, really) that are applicable with regards to the
// data. These are grouped together and we then generate a common modified
// ballot for them, so that we can cache. Then we go through them and check
// if they pass or fail the criteria, set these methods aside, and do it again
// with new data. If we go more than num_nc_iters without having a single 
// new method turn applicable, we give up.
// If twotests could have a function that would coax data to fit a certain 
// outcome (e.g. set a given winner for monotonicity), then we could speed
// up this part by a lot.
void twotest_engine::run_single_test(const vector<ordering> & base_outcomes,
		const list<ballot_group> & original_ballots, int num_candidates,
		int num_nc_iters, const twotest * test, const 
		vector<election_method *> & methods_to_test, const
		vector<method_test_info> & compliance_data,
		bool skip_already_false) {

	// Perhaps pass this later. We'll see. Profile first, and only then
	// optimize.
	vector<ternary> status(methods_to_test.size(), TINAPP);

	int methods_left = methods_to_test.size();

	// If we're supposed to skip those that are already false, incorporate
	// that data into status s.th. we'll skip them automatically.
	
	int counter;

	if (skip_already_false) {
		for (counter = 0; counter < prior_compliance_data.size();
				++counter)
			if (!prior_compliance_data[counter].passes_so_far) {
				status[counter] = TFALSE;
				--methods_left;
			}
	}

	int num_uninterrupted = 0;

	// Only the base data is set here, modified ballots will be set later.
	// May need optimization and direct setters in the test later.
	disproof potential_disproof;
	potential_disproof.unmodified_ballots = original_ballots;

	// As long as we have some methods left and haven't exceeded 
	// num_nc_iters...
	cache_map mod_cache;

	while (methods_left > 0 && num_uninterrupted < num_nc_iters) {
		++num_uninterrupted;

		// Generate some data for the test.
		potential_disproof.modification_data = test->
			generate_aux_data(input, num_candidates);

		// Determine if there's at least one method for which the
		// data is applicable. If not, no need to waste time trying
		// to make a modified ballot.
		bool one_applicable = false;
		for (int counter = 0; counter < status.size() && 
				!one_applicable; ++counter) {

			// If it's already determined, no need to go further.
			if (status[counter] != TINAPP)
				continue;

			// If not applicable, continue.
			// NOTE: Do remember how this will come into play
			// wrt methods that have been determined as false.
			// Counter still needs to take this into account, so
			// if #0 is false, the first method we check (which
			// would be #1, but zeroth in sequence) must have
			// its base outcome set at base_outcome[1].
			if (!applicable(base_outcome[counter], data, true))
				continue;

			// If we go here, it was applicable, so break.
			one_applicable = true;
		}

		// If nothing was applicable, try again.
		if (!one_applicable)
			continue;

		// Otherwise, generate the modified ballot set and start 
		// testing! They might still not be applicable, but we don't 
		// know that yet.
		pair<bool, list<ballot_group> > arrangement = 
			test->rearrange_ballots(original_ballots, 
					num_candidates, data);

		// If we couldn't make a modified ballot out of it, try again.
		if (!arrangement.first)
			continue;

		int first = counter - 1;
		mod_cache.clear();

		// Now check if it fails/passes. For the first method, we must
		// load the potential disproof, so do that.
		potential_disproof.modified_balltos = arrangement.second;
		test->set_disproof(potential_disproof);

		// For all the tests that remain, check if they return something
		// different from TINAPP. If so, mark them as such.
		for (counter = first; counter < status.size() && 
				methods_left > 0; ++counter) {
			if (status[counter] != TINAPP)
				continue;

			test->set_unmodified_ordering(base_outcomes[counter]);

			ternary f = test->pass_last(methods_to_test[counter],
					num_candidates, true);

			if (f != TINAPP) {
				status[counter] = f;
				--methods_left;
				// Update prior compliance data
				++prior_compliance_data[counter].iters_run;
				if (f == TFALSE) {
					prior_compliance_data[counter].
						passed_so_far = false;
					prior_compliance_data[counter].
						crit_disproof = 
						test->get_disproof();
				}
			}
		}
	}
}

bool twotest_engine::run_tests(int iterations) {

	cache_map orig_cache, mod_cache;

	list<election_method *> used, unused;
	list<twotest *> applicable;

	int counter, sec, tri;

	for (counter = 0; counter < iterations; ++counter) {

		// Generate a base ballot set.

		int num_voters = bracket_random(min_num_voters, max_num_voters);
		int num_cands = bracket_random(min_num_cands, max_num_cands);

		list<ballot_group> base_ballot = generator->generate_ballots(
				num_voters, num_cands);

		// Group together all tests that can build upon this 
		// ballot_set. To do so, we must get the outcome for every
		// method for the base ballot. TODO? Make a function that
		// coaxes data so it can work with the outcome that the method
		// actually provided?

		// TODO: Winner_only based on whether we need more data than the
		// winner or not.

		list<ordering> base_outcomes;
		orig_cache.clear();

		for (sec = 0; sec < methods.size(); ++sec)
			base_outcomes.push_back(methods[sec]->elect(base_ballot,
						num_cands, orig_cache, false));

		// Get a list of all tests that can use this ballot set,
		// as well as the methods for which that test's data settings
		// are applicable.
		
		list<tests> workable;
		list<list<methods> > applicable_tests;
		for (sec = 0; sec < tests.size(); ++sec) {
			list<method> applicable_this;
			for (list<ordering>::const_iterator binc = 
					base_outcomes.begin(); 
					binc != base_outcomes.end(); ++binc)
				if (tests[sec]->applicable(*binc, data, true))
					applicable_this.push_back(*binc);

			if (!applicable_this.empty()) {
				workable.push_back(tests[sec]);
				applicable_tests.push_back(applicable_this);
			}
		}

		// Idea: For each test in can_work, generate modified ballot and
		// check which methods it's applicable to. Test and mark those
		// methods as done. Repeat until all are done or we give up (as
		// applicable() may not be accurate), clearing cache in between.
		// Repeat for all workable tests, clearing cache in between.
