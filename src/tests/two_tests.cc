// Two-tests for criterion compliance.

#include "two_tests.h"

// Should this be a pseudo-singlewinner method? Nah, because then we lose the
// capacity of picking number of seats as needed.
ordering twotest::synthesize_single_winner(const list<int> & mw_council, 
		int num_candidates) const {

        vector<bool> members(num_candidates, false);
        list<int>::const_iterator lpos;
        for (lpos = mw_council.begin(); lpos != mw_council.end(); ++lpos)
		members[*lpos] = true;

        ordering unmod;
        for (int counter = 0; counter < num_candidates; ++counter)
                if (members[counter])
                        unmod.insert(candscore(counter, 1));
                else    unmod.insert(candscore(counter, 0));

	return(unmod);
}


ternary twotest::pass(const election_method * base, 
		const list<ballot_group> & input,
		int num_candidates, cache_map * unmod_cache,
		cache_map * mod_cache) {

	disproof_out.complete = false;
	disproof_out.unmodified_ballots = input;
	disproof_out.unmodified_ordering.clear();

	// Get some random data.
	pair<bool, list<ballot_group> > arrangement;
	arrangement.first = false;

	disproof_out.modification_data = generate_aux_data(input,
			num_candidates);

	// Get ordinary ordering and check if it's applicable.
	disproof_out.unmodified_ordering = base->elect(disproof_out.
			unmodified_ballots, num_candidates, unmod_cache,
			winner_only());
	if (!applicable(disproof_out.unmodified_ordering,
				disproof_out.modification_data, true))
		return(TINAPP);

	// Generate the modified ballots.
	arrangement = rearrange_ballots(input, num_candidates,
			disproof_out.modification_data);
		
	// No luck? Give up and call the original inapplicable.
	// Note that adding a loop before this would distort the
	// probability calcs.
	if(!arrangement.first) 
		return(TINAPP);

	// Otherwise, set the modified ballot list.
	disproof_out.modified_ballots = arrangement.second;

	// Now "pass last".

	return(pass_last(base, num_candidates, true, unmod_cache, mod_cache));
}

ternary twotest::pass(const election_method * base,
		const list<ballot_group> & input,
		int num_candidates) {

	return(pass(base, input, num_candidates, NULL, NULL));
}

ternary twotest::pass_specd(const election_method * base,
		const disproof & to_test, int num_candidates,
		cache_map * unmod_cache, cache_map * mod_cache) {

	disproof_out = to_test;
	
	// Generate the modified ballots.
	pair<bool, list<ballot_group> > arrangement;
	arrangement = rearrange_ballots(disproof_out.unmodified_ballots, 
			num_candidates, disproof_out.modification_data);

	if(!arrangement.first)
		return(TINAPP);

	// Otherwise, set the modified ballot list.
	disproof_out.modified_ballots = arrangement.second;

	return(pass_last(base, num_candidates, false, unmod_cache, mod_cache));
}

ternary twotest::pass_last(const election_method * base, int num_candidates,
		bool unmod_already_set, cache_map * unmod_cache, 
		cache_map * mod_cache) {
	// We assume the data structure (ballot and modified, and data) have
	// been filled out.

	/*map<int, string> fakecand;
	        int sec;

	        string f = "!";
	        for (int counter = 0; counter < 26; ++counter) {
	                f[0] = (char)('A' + counter);
	                fakecand[counter] = f;
	        }

	cout << "DEBUG: Checking " << endl;
	ballot_tools btools;
	vector<string> fv = btools.ballots_to_text(disproof_out.
			unmodified_ballots, fakecand, false);
	copy(fv.begin(), fv.end(),
			ostream_iterator<string>(cout, "\n"));*/

	// Produce the orderings
	if (!unmod_already_set)
		disproof_out.unmodified_ordering = base->elect(disproof_out.
				unmodified_ballots, num_candidates, unmod_cache,
				winner_only());
	// Check if it's applicable
	if (!applicable(disproof_out.unmodified_ordering,
				disproof_out.modification_data, true))
		return(TINAPP);

	// Get the modified ordering.
	disproof_out.modified_ordering = base->elect(disproof_out.
			modified_ballots, num_candidates, mod_cache, 
			winner_only());
	// Doesn't this let it get off free? Well, in another sense, the
	// scenarios are symmetric and therefore if we're going to check one,
	// we should also check the other (or do a xor).
	if (!applicable(disproof_out.modified_ordering,
				disproof_out.modification_data, false))
		return(TINAPP);

	// We can't check applicable(modified) here because it might be
	// relevant - for instance, reversal symmetry with disallow ties
	// should still return false if original has no tie and reversal has.

	disproof_out.complete = true;

	// Then just check if they pass!
	if (pass_internal(disproof_out.unmodified_ordering, 
				disproof_out.modified_ordering, 
				disproof_out.modification_data, num_candidates))
		return(TTRUE);
	else	return(TFALSE);
}

ternary twotest::pass_last(const election_method * base, int num_candidates) {

	return(pass_last(base, num_candidates, false, NULL, NULL));
	
}

// This function is for checking many functions at once against the test. It
// makes use of caching to speed up methods that relate to each other (e.g.
// Smith,* will only calculate the Smith set once).
//	Base_outcomes is the outcomes for the different methods for the original
//	 ballot group. It should be listed in the same order as methods_to_test,
//	 i.e. original_ballots[x] = methods_to_test[x]'s outcome.
//	Original_ballots is the original ballot set.
//	Num_nc_iters is the number of times to try finding a modified ballot
//	 group that is applicable before giving up. The counter is reset every
//	 time at least one of the methods are applicable.
//	Compliance_data is the pass/fail/number of tries attempted data for
//	 each method.
//	If skip_already_false is true, we skip all methods for which a disproof
//	 has already been found (since one disproof is enough to prove the
//	 method fails the criterion in question). If it's false, we don't
//	 (e.g. for statistical purposes).

// The logic involves starting with all methods unknown. Then we try to make
// a modified ballot where the outcomes are applicable to the test (e.g.
// changes the winner in monotonicity tests) and mark the methods for which
// that combination is applicable so they won't be tested further. The loop ends
// when all methods have been tested or we exhaust the number of tries specified
// in num_nc_iters without being able to find a ballot set applicable to at
// least one method.

// Possible TODO: pass mod_cache instead of making it. Depends on how much time
// it takes to create a new map vs clearing it.
bool twotest::pass_many(const vector<ordering> & base_outcomes,
		const list<ballot_group> & original_ballots, 
		int num_candidates, int num_nc_iters, 
		const vector<const election_method *> & methods_to_test,
		vector<method_test_info> & compliance_data, 
		bool skip_already_false) {

	//cout << "Debug: test is " << name() << endl;

	// Start out knowing nothing about the methods.
	vector<ternary> status(methods_to_test.size(), TINAPP);

	size_t counter;
	int methods_left = methods_to_test.size();

	// If we're supposed to skip those that are already false, incorporate
	// that data into status s.th. we'll skip them automatically.
	if (skip_already_false) {
		for (counter = 0; counter < compliance_data.size(); ++counter)
			if (!compliance_data[counter].passes_so_far) {
				status[counter] = TFALSE;
				--methods_left;
			}
	}

	int num_uninterrupted = 0; // iters since we last tested something.
	
	cache_map mod_cache;
	disproof_out.unmodified_ballots = original_ballots;

	while (methods_left > 0 && num_uninterrupted < num_nc_iters) {
		//cout << "-dbg-" << endl;
		++num_uninterrupted;

		// Generate some data for the test.
		disproof_out.modification_data = generate_aux_data(
				original_ballots, num_candidates);

		// Determine if there's at least one method for which the data
		// is applicable. If not, no need to waste time trying to make
		// a modified ballot.
		// Having a function to coax the data to fit a certain base
		// ordering would be very useful here.
		bool one_applicable = false, all_discovered = true;
		for (counter = 0; counter < status.size() && !one_applicable;
				++counter) {

			if (status[counter] != TFALSE)
				all_discovered = false;

			// If it's already determined, no need to go further.
			if (status[counter] != TINAPP)
				continue;

			// If not applicable, continue.
			if (!applicable(base_outcomes[counter], 
						disproof_out.modification_data,
					       	true))
				continue;

			// If we get here, it was applicable, so break;
			one_applicable = true;
		}

		// If nothing was applicable, try again with some other data.
		if (!one_applicable)
			continue;

		// If everything has been discovered and we have been told to
		// not find further disproofs where disproofs have already been
		// found, get outta here.
		if (all_discovered && skip_already_false)
			return(false);

		// Otherwise, generate the actual modified ballot group and
		// start testing! It might still not be applicable or even 
		// possible,  but we don't know that yet.
		pair<bool, list<ballot_group> > arrangement =
			rearrange_ballots(disproof_out.unmodified_ballots,
					num_candidates, 
					disproof_out.modification_data);

		// If we couldn't make a modified ballot out of it, try again
		// with other data. (This could happen, for instance, if we're
		// trying to make a monotonicity failure and everybody ranks
		// the winner first - then he can't be raised anywhere)
		if (!arrangement.first)
			continue;

		// We're all set, so load the modified ballots and set the
		// starting point...
		disproof_out.modified_ballots = arrangement.second;
		int first = counter-1;

		// We need to clear the cache since we've altered the modified
		// ballots; failure to do so could make it match a previous
		// modified ballot outcome, which would be bad.
		mod_cache.clear();

		// And then, for all tests that remain, check if they return
		// something other than TINAPP. If so, we've tested them, so
		// mark them as such.
		for (counter = first; counter < status.size() && 
				methods_left > 0; ++counter) {
			if (status[counter] != TINAPP)
				continue;

			// Would this be more elegant if we replaced it with
			// a cache lookup? Yes, but also slower.
			disproof_out.unmodified_ordering = 
				base_outcomes[counter];

			// TODO: Fix bug here. Adding mod_cache makes the system
			// report that Plurality fails mono-raise. The problem
			// seems to be a hit that shouldn't have happened.
			// DONE?
			ternary result = pass_last(methods_to_test[counter],
					num_candidates, true, NULL, &mod_cache);

			if (result != TINAPP) {
				//cout << methods_to_test[counter]->name() << endl;
				status[counter] = result;
				--methods_left;

				// Update compliance data.
				++compliance_data[counter].iters_run;
				if (result == TFALSE) {
					cout << "result for " << methods_to_test[counter]->name() << " and " << name() << " is " << result << " at iter " << compliance_data[counter].iters_run << endl;
					compliance_data[counter].
						passes_so_far =	false;
					compliance_data[counter].
						crit_disproof =	disproof_out;
				}
			}
		}
		//cout << methods_left << endl;
	}

	//cout << "---" << endl;

	return(true);
}

// TODO BLUESKY: Some way of doing multiwinner twotests, e.g. DPC.
/*ternary twotest::pass_multiwinner_last(const multiwinner_method * base_mw,
		int council_size, int num_candidates) {
	// Same as above, only we use a trick where the winning council comes
	// first and all others tie for second.
	// TODO: Ties must be allowed, and only winners/losers be counted.
	// Some methods (Schulze STV etc) provide for a full ranking of every
	// possible council, but that's exponential and so we don't support it.
	
	list<int> unmodified_council = base_mw->get_council(council_size,
			num_candidates, disproof_out.unmodified_ballots);
	disproof_out.unmodified_ordering = synthesize_single_winner(
			unmodified_council, num_candidates);
	if (!applicable(disproof_out.unmodified_ordering,
				disproof_out.modification_data, true))
		return(TINAPP);

	// Modified council
	list<int> modified_council = base_mw->get_council(council_size,
			num_candidates, disproof_out.modified_ballots);
	disproof_out.modified_ordering = synthesize_single_winner(
			modified_council, num_candidates);
	if (!applicable(disproof_out.modified_ordering,
				disproof_out.modification_data, false))
		return(TINAPP);

	disproof_out.complete = true;

	if (pass_internal(disproof_out.unmodified_ordering,
				disproof_out.modified_ordering,
				disproof_out.modification_data,
				num_candidates))
		return(TTRUE);
	else	return(TFALSE);
}

ternary twotest::pass_multiwinner(const multiwinner_method * base_mw,
		const list<ballot_group> & input, int council_size, 
		int num_candidates) {

	// Second verse, same as the first. (See above)
	// Maybe making a surrogate single winner method would be best
	// after all, then just integrate it into twotest...
	
        disproof_out.complete = false;
        disproof_out.unmodified_ballots = input;

        // Get some random data.
        pair<bool, list<ballot_group> > arrangement;
        arrangement.first = false;

        disproof_out.modification_data = generate_aux_data(input,
                        num_candidates);

        // Generate the modified ballots.
        arrangement = rearrange_ballots(input, num_candidates,
                        disproof_out.modification_data);

        // No luck? Give up and call the original inapplicable.
        // Note that adding a loop before this would distort the
        // probability calcs.
        if(!arrangement.first)
                return(TINAPP);

        // Otherwise, set the modified ballot list.
        disproof_out.modified_ballots = arrangement.second;

        // Now "pass last".
        return(pass_multiwinner_last(base_mw, council_size, num_candidates));
}*/
