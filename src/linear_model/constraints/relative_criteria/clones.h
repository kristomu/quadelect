#pragma once

#include "../relative_criterion.h"

// Relative criterion constraint generator for independence of clones.
// The generator imposes a clone constraint linking before and after,
// so that the given candidate in before is cloned into two candidates in
// after.

class clone_const : public relative_criterion_const {
	private:
		std::vector<int> after_as_before;

		// Check that it's valid.
		bool check_after_as_before(
			std::vector<int> after_as_before_in) const;

		void set_after_as_before(
			std::vector<int> after_as_before_in) const;
	
	protected:
		bool permissible_transition(
			const std::vector<int> & before_permutation,
			const std::vector<int> & after_permutation) const;

	public:
		// after_as_before is an index where after_as_before[i] gives
		// what after candidate corresponds to what before candidate.
		// E.g. after_as_before = {0, 0, 1, 2} means that we're cloning
		// three candidates into four, and the after candidates A, B, C, D
		// correspond to the before candidates A, B, C, respectively.
		clone_const(std::vector<int> after_as_before_in);

		// Clones the first candidate into the first and all the new
		// ones. E.g. before = 3, after = 5, clones A in the three-candidate
		// election into A, D, and E in the five.
		clone_const(int numcands_before_in, int numcands_after_in);
};