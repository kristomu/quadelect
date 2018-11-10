#ifndef _COMP_ELIGIBLE
#define _COMP_ELIGIBLE

// We ultimately want to construct an election method that is monotone in
// various ways, and we can specify an election method for k candidates as
// a combination of a number of gen_custom_functions, one for each canonical
// scenario.

// To determine whether a candidate combination of gen_custom_functions is
// suitable for our purposes (i.e. whether it passes monotonicity), it seems
// the obvious way is to try every possible election method, i.e. every
// possible k-combination of gen_custom_functions. This is a problem if
// we have tens of millions of candidate gen_custom_functions. Even with
// four candidates, and even if we only care about elections with a Smith
// set of all four candidates, tens of millions choose four is still an
// extreme number.

// However, we can do better by a meet-in-the-middle approach. Suppose we
// have a monotonicity scenario of the following form:
// Base election is eA, then we improve A's lot on it and we get eA'.
// eA from the perspective of candidate A is scenario s(eA, A).
// and the following holds:
// s(eA, A) = s(eA', A)
// s(eA, B) = s(eA', B)

// Applying meet-in-the-middle gets us from having to evaluate n^4 functions
// to 4 * n^2 per election example. But n^2 can still be infeasibly large if
// n is on the order of millions, so a better strategy is needed.

// We can combine the meet-in-the-middle approach with memoization. We can
// keep a lookup table of algorithm numbers so that lookup[0] is the first
// algorithm number we may want to use as a part of a final election method,
// and so on up until lookup[n-1]. For each election example (A, A', etc), we
// can let memoization[A][k] store the result of the gen_custom_function
// corresponding to the algorithm number in lookup[k]. Since the lookup table
// is contiguous, this makes the memoization array O(n) instead of
// O(max algorithm number).

// Since we can't afford to clean the memoization array between each election
// example (in particular not when there are only a few methods left), the
// memoization array should be of a pair: a term counter, and a score. If
// the term counter is below the current term, the value is outdated and must
// be refreshed. Otherwise it is valid and can be directly used.

// The meet-in-the-middle strategy can be extended to extend three-candidate
// election methods into four-candidate election methods (if we assume ISDA).

// I'm going to put most of the logic into compositor.cc, and then
// refactor back into here once I find out how the dependencies should be
// arranged.

#include "scenario.h"
#include "../rpn_evaluator.h"
#include <set>
#include <map>

class eligibility_status {
	public:
		// For monotonicity to pass, we need that increasing A's lot never
		// makes him lose, and that at least once, it makes him go from a
		// loser to a winner. The first is an AND criterion, but the latter
		// is an OR criterion and so has to be handled like this. More info
		// later.
		algo_t first, second;
		bool any_strict_inequality;

		eligibility_status() { any_strict_inequality = false; }
};


class eligibility_table {
	public:
		std::list<eligibility_status> eligibles;
};

class eligibility_tables {
	public:
		std::map<std::pair<copeland_scenario, copeland_scenario>,
			eligibility_table> table_per_scenario_tuple;

};

class memoization_entry {
	public:
		int term;
		double score; // alternately vector<double>...

		memoization_entry() { term = -1; score = -1; }
};

#endif
