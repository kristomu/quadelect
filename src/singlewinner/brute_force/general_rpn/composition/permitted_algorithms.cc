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

// Then we can calculate the score according to every gen_custom_function
// applied on eA to get these functions' scores for [eA, s(eA, A)], and we
// can similarly get the score according to every gen_custom_function
// applied on eA with candidates rotated so what used to be B is now A,
// to get all functions' scores for [eA, s(eA, B)]. 
// In a completely analogous manner, we can get [eA, s(eA', A)] and 
// [eA, s(eA', B)] for every method.
// Then we don't have to do any more calculations: any pair of 
// gen_custom_functions X, Y, so that its score for A on eA (determined by
// applying gen_custom_function X) is greater than its score for B on eA
// (determined by applying gen_custom_function Y), but where its score for
// A on eA' is less than its score for B on eA', fails monotonicity, and
// no four-combination with X assigned to s(eA, A) and Y assigned to s(eA, B)
// can be valid, no matter what the other two gen_custom_functs are.

// This strategy can be extended to extend three-candidate election methods
// into four-candidate election methods (if we assume ISDA).

// What we need is a table of compatible n-tuples of gen_custom_functions,
// initially only pairs. Each of these tables should be contained in another
// data structure, indexed by pairs of scenarios, so that we can quickly
// update the tables every time we run through a new monotonicity example.
// Furthermore, the tables should also have sets of "every method still
// present" for each column of the table, so that we know what methods to
// test the next election methods on.

// Finally, updating the table entries should be easy (constant access)
// since we're going to do it a lot, and we need a particular data structure
// for each entry to keep track of whether they're still eligible. (More on
// that in another file.)

#include

class eligibility_status {
	public:
		// For monotonicity to pass, we need that increasing A's lot never
		// makes him lose, and that at least once, it makes him go from a
		// loser to a winner. The first is an AND criterion, but the latter
		// is an OR criterion and so has to be handled like this. More info
		// later.
		bool any_strict_inequality;

		eligibility_status() { any_strict_inequality = false; }
};


class eligibility_table {
	private:
		std::map<algo_t, std::set<algo_t> > eligible;
		std::vector<std::map<algo_t, int> > eligibles_kth_column;
		std::map<std::pair<algo_t, algo_t>, eligibility_status> status;

	public:
		void mark_eligible(algo_t first, algo_t second, 
			bool strict_inequality);
		void mark_ineligible(algo_t first, algo_t second);

		const std::vector<std::map<algo_t, int> > & 
			get_eligibles_kth_column(int k) const {
			return eligibles_kth_column[k];
		}
};

void eligibility_table::mark_eligible(algo_t first, algo_t second,
	bool strict_inequality) {

	if (!eligible[first].contains(second)) {
		eligible[first].insert(second);
		eligibles_kth_column[0][first]++;
		eligibles_kth_column[1][second]++;
		status[std::pair<algo_t, algo_t>(first, second)] = 
			eligibility_status()
	}
}