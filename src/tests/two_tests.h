// Two-tests for criterion compliance.
// The general framework of this is:
// 	- the test accepts a "standard ballot". It then rearranges it in some
// 	  way (which may or may not depend on chance), and compares the 
// 	  outcomes for the two ballots. If they pass a certain check, the
// 	  test passes, otherwise it fails.

// This is in contrast to a one-test (for set membership, etc), where the same
// ballot group is run through two methods, and then compared according to some
// auxiliary check.

// To demonstrate how that works, we'll also construct a two-test class to check
// for Reversal Symmetry compliance.

// TODO: Some way of signaling, in an election method, "this was because of a 
// tie and we made use of random information to break it".
// TODO BLUESKY: Set versions of these. E.g mono-raise: raising a candidate
// leads to a disjoint set (then some method that elects from the set can't 
// possibly pass the criterion)

// The way this is done, it takes way too long. The construction of the unmod
// and modified ballots eats up time even before we check whether they're
// inapplicable, etc...
// It might be better to have unmod, to_add, to_remove. In most cases, the
// modification is much shorter than the actual ballot. Handling to_remove might
// be a bit difficult, however.

// DONE?: Update to handle cache. 
// Also consider removing some of the pass functions here as they're duplicated
// in tte. We might want to refactor the entire thing.

// BLUESKY: A "Refine" function: given two ballots that together exhibit 
// failure, make one closer and closer to the other until the minimum increment
// required to cause the failure. That could give information about how the
// failure actually happens.

// (Even further, one could imagine a measure of how close one is to failure,
//  and use a black box optimization method to construct a failure in very rare
//  cases, e.g. mono-add-top failure in Smith,minmax(margins).)

#ifndef _VOTE_TWOTEST
#define _VOTE_TWOTEST

//#include "../multiwinner/methods.cc"
#include "../ballot_tools.h"
#include "../singlewinner/method.h"
#include <iterator>
#include <vector>

#include "disproof.h"

// INAPP if it's not possible to do a check using that ballot set or disproof.
enum ternary {TFALSE = -1, TINAPP = 0, TTRUE = 1};

using namespace std;

class twotest {
	private:
		ordering synthesize_single_winner(const list<int> &
				mw_council, int num_candidates) const;

	protected:

		// Pick a candidate to move for mono-raise, a ranking to add
		// for mono-add-top, etc., and then seed for random raising.
		virtual vector<int> generate_aux_data(
			const list<ballot_group> & input,
			int numcands) const { return (vector<int>(0)); }

		// Always provides the same modification with the same data.
		// The boolean is false if we couldn't alter the ballots
		// according to the data (i.e. symmetric ballots in reversal).
		virtual pair<bool, list<ballot_group> > rearrange_ballots(
				const list<ballot_group> & input,
				int numcands,
				const vector<int> & data) const = 0;

		// We give access to data so that mono-raise with a given
		// candidate to raise and only winners considered can return
		// false if the candidate in question isn't ranked top in
		// the social ordering.
		virtual bool applicable(const ordering & check,
				const vector<int> & data, bool orig) const = 0;

		virtual bool pass_internal(const ordering & original, 
				const ordering & modified, 
				const vector<int> & data, 
				int numcands) const = 0;

		// For disproof.
		disproof disproof_out;

		virtual string explain_change_int(const vector<int> & data,
				const map<int, string> & cand_names) const = 0;

	public:
		
		// Check a disproof.
		// For all these pass_* methods, unmod_cache is the cache for
		// the unmodified ballots (base case), and mod_cache is the
		// cache for the modified ballots. Functions that construct
		// modified ballots *will* wipe mod_cache.
		virtual ternary pass_specd(const election_method * base,
				const disproof & to_test, int num_candidates,
				cache_map * unmod_cache, cache_map * mod_cache);

		virtual ternary pass(const election_method * base,
				const list<ballot_group> & input,
				int num_candidates, cache_map * unmod_cache,
				cache_map * mod_cache);
		ternary pass(const election_method * base,
				const list<ballot_group> & input,
				int num_candidates);
		// Run a test with the same ballot and modified as last time.
		// If unmod_last_set is true, we don't try to find the outcome
		// for the original ballots, but if it is false, we do so.
		virtual ternary pass_last(const election_method * base,
				int num_candidates, bool unmod_last_set,
				cache_map * unmod_cache, cache_map * mod_cache);
		ternary pass_last(const election_method * base,
				int num_candidates);

		// See comments by the implementation to see how to use this
		// function. The function is void, but we update 
		// compliance_data.
		// Returns false if/when all methods have disproofs if
		// skip_already_false is true, otherwise returns true.
		bool pass_many(const vector<ordering> & base_outcomes,
				const list<ballot_group> & original_ballots,
				int num_candidates, int num_nc_iters, const 
				vector<const election_method *> & 
				methods_to_test, 
				vector<method_test_info> & compliance_data, 
				bool skip_already_false);

		/*virtual ternary pass_multiwinner_last(const multiwinner_method *
				mwbase, int council_size, int num_candidates);

		virtual ternary pass_multiwinner(const multiwinner_method *
				mwbase, const list<ballot_group> & input,
				int council_size, int num_candidates);*/
		// Pass_multiwinner. Also enable winner_only in the case
		// of MW, since it's just [winning council ] > [all others].

		// Note that this is always updated, even when there's no
		// actual disproof (the return value is TRUE). It's called
		// disproof because it only really "proves" anything when
		// the method fails.
		disproof get_disproof() const { return(disproof_out); }
		void set_disproof(const disproof & in) { disproof_out = in; }

		// Some more setters are required for cache-aware test engines.
		// Uhglee!
		void set_unmodified_ordering(const ordering & in) {
			disproof_out.unmodified_ordering = in;
		}

		virtual string name() const = 0;

		// Return true if the test only needs to know who the winners
		// are, otherwise false.
		virtual bool winner_only() const = 0;
		
		string explain_last_change(const map<int, string> & 
				cand_names) const {
			return(explain_change_int(disproof_out.
						modification_data,
						cand_names));
		}

		string explain_change(const disproof & disproof_in,
				const map<int, string> & cand_names) const {
			return(explain_change_int(disproof_in.
						modification_data,
						cand_names));
		}
};

#endif
