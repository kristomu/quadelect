// Testing engine for relative criteria (two_tests). A relative criterion
// is a constraint on the outcomes of two ballot sets that are similar in
// some way.

#include "twotest.h"

void twotest_engine::set_generator(pure_ballot_generator * gen_in) {
        generator = gen_in;
}

void twotest_engine::set_maxiters(unsigned int max_iters) {
        num_iterations = max_iters;
}

void twotest_engine::set_num_voters(int minimum, int maximum) {
        min_num_voters = minimum;
        max_num_voters = maximum;
}

void twotest_engine::set_num_candidates(int minimum, int maximum) {
        min_num_candidates = minimum;
        max_num_candidates = maximum;
}

// Test-major, so we must add a new m_t_i to each vector we have.
void twotest_engine::add_method(const election_method * method_in) {
	methods.push_back(method_in);
	for (size_t counter = 0; counter < method_pass_status.size(); ++counter)
		method_pass_status[counter].push_back(method_test_info());
}

// Test-major, so add a new vector<method_test_info> to those we already have.
void twotest_engine::add_test(twotest * twotest_in) {
	
	tests.push_back(twotest_in);
	method_pass_status.push_back(vector<method_test_info>());
}



twotest_engine::twotest_engine() {
        generator = NULL;
        set_maxiters(1024);
        set_num_voters(1, 100);
        set_num_candidates(1, 10);
}

// TODO: bool skip_already_false
twotest_engine::twotest_engine(unsigned int max_iters,
                int min_voters, int max_voters, int min_cands, int max_cands) {
        generator = NULL;
        set_maxiters(max_iters);
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
/*void twotest_engine::run_single_test(const vector<ordering> & base_outcomes,
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
}*/

// BLUESKY: Handle multi-way criterion failures: e.g. in the original case,
// a method ranks A and B equal first, then we check if we can find an example
// both assuming A > ... and B > ...; if so, we have a counterexample no matter
// who wins and so the method fails it.

bool twotest_engine::run_tests(int iterations, rng & random_source) {

	cache_map orig_cache;

	size_t counter, sec;
	vector<ordering> base_outcomes(methods.size());

	// Generate a base ballot set.

	int num_voters = random_source.irand(min_num_voters, max_num_voters);
	int num_cands = random_source.irand(min_num_candidates, 
			max_num_candidates);

	list<ballot_group> base_ballots = generator->generate_ballots(
			num_voters, num_cands, random_source);

	// If every test is winner_only, then only count winners, otherwise 
	// require a full ordering. The reasoning would be that doing full 
	// ordering alone makes winner-only methods happy, and if we have
	// at least one full ordering test, we're going to have to get the
	// full ordering anyway. 

	// A possible further optimization would involve checking if a test
	// is to be run at all. If all methods fail it and we're skipping
	// falses, then it should not be counted below, nor should it even be 
	// run... but eh, is it worth it?

	bool winner_only = true;

	for (counter = 0; counter < tests.size() && winner_only; ++counter)
		winner_only = winner_only && tests[counter]->winner_only();

	// Determine the base outcomes.
	for (sec = 0; sec < methods.size(); ++sec)
		base_outcomes[sec] = methods[sec]->elect(base_ballots,
				num_cands, orig_cache, winner_only);

	// For all tests, check if we can make modified ballots, and if so, 
	// test them. This is done within twotest - see the spec for the 
	// function for more info about how it works.
	
	// TODO: Perhaps let skip_already_false be given by the user.
	sec = 0;

	bool saw_change = false;

	for (vector<twotest *>::iterator ltpos = tests.begin(); ltpos !=
			tests.end(); ++ltpos)
		saw_change |= (*ltpos)->pass_many(base_outcomes, base_ballots, 
				num_cands, iterations, methods, 
				method_pass_status[sec++], true);

	return(saw_change); // What's this for? Oh, it should return false if 
	// all tests are false and skip_already_false is true, so that a timed
	// test can bail out at that point. TODO, do that.
	
}
