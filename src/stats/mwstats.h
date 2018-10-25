// Taken from the non-cache quadelect. I'll port the rest at some later point,
// once singlewinner works well enough.

#ifndef __VOTE_STATS_MW
#define __VOTE_STATS_MW

#include "stats.h"
#include "../multiwinner/methods.h"

using namespace std;

class multiwinner_stats : public stats {

	private:
		multiwinner_method * mw_method;

	public:
		// We can set name to arbitrary because we don't use it.
		// A more abstract base class would fix this, but bah.
		multiwinner_stats(stats_type norm_type, multiwinner_method *
				method_in, bool only_sum) : stats(norm_type, 
					"MWSTATS", only_sum) {
					mw_method = method_in;}
		multiwinner_stats(stats_type norm_type, multiwinner_method *
				method_in) : stats(norm_type, "MWSTATS") {
			mw_method = method_in;}

		// No direct add result because that depends on error type.
		// Perhaps refactor error type to use classes instead of
		// #defines, later. But we'll use the same result for multiple
		// checks, so it's still not as useful unless we can cache,
		// which will have to wait for "clues".

		string get_name() const { return(mw_method->name()); }

		const multiwinner_method * method() { return(mw_method); }
};

#endif
