#include "clones.h"
#include <algorithm>
#include <numeric>
#include <assert.h>

bool clone_const::permissible_transition(
	const std::vector<int> & before_permutation,
	const std::vector<int> & after_permutation) const {

	// If the before permutation is as long as or longer than the after,
	// return false.
	// We only support cloning from a shorter number of candidates to a
	// longer number of candidates, so that we don't get users mixing
	// cloning from longer to shorter and shorter to longer in the same
	// transition.

	if (before_permutation.size() >= after_permutation.size()) {
		return false;
	}

	// Now use the after_as_before array to turn after_permutation into
	// something in terms of the candidates in before_permutation, removing
	// duplicates as we go. The transition is permissible iff the result is
	// exactly the before permutation. After permutations that aren't clone
	// situations at all will end up too long after deduplicating.

	std::vector<int> transformed_after;
	bool first = true;
	int last_candidate = -1;

	for (int cand: after_permutation) {
		int before_equivalent = after_as_before[cand];

		if (first || before_equivalent != last_candidate) {
			transformed_after.push_back(before_equivalent);
		}
		first = false;
		last_candidate = before_equivalent;
	}

	return before_permutation == transformed_after;
}

clone_const::clone_const(std::vector<int> after_as_before_in) :
	relative_criterion_const
		// Number of candidates before cloning: maximum value in a_a_b_i
		(*std::max_element(after_as_before_in.begin(),
			after_as_before_in.end()),
		// Number of candidates after cloning: length of after_as_before_in
		after_as_before_in.size()) {

	after_as_before = after_as_before_in;
}

clone_const::clone_const(int before_numcands_in, int after_numcands_in) :
	relative_criterion_const(before_numcands_in, after_numcands_in) {

	assert (before_numcands_in < after_numcands_in);

	// Now every after candidate is a clone of A...
	after_as_before = std::vector<int>(after_numcands_in, 0);
	// ... but we don't want the before-candidates to be clones of A.
	std::iota(after_as_before.begin(),
		after_as_before.begin() + before_numcands_in, 0);
}