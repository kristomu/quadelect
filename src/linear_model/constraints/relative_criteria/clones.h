#pragma once

#include "../relative_criterion.h"

// Relative criterion constraint generator for independence of clones.
// The generator imposes a clone constraint linking before and after,
// so that the given candidate in before is cloned into two candidates in
// after.

// TODO: Somehow find out how to specify the three types of independence of
// clones:
//		- Teaming: Cloning A shouldn't help A
//		- Vote-splitting: Cloning A shouldn't harm A
//		- Crowding: Cloning B shouldn't help/harm A

class clone_const : public relative_criterion_const {
	private:
		std::vector<int> after_as_before;

		// Check that it's valid.
		bool check_after_as_before(
			std::vector<int> after_as_before_in) const;

		void set_after_as_before(
			std::vector<int> after_as_before_in) const;

	protected:
		virtual std::vector<int> get_after_as_before() const;

		bool permissible_transition(
			const std::vector<int> & before_permutation,
			const std::vector<int> & after_permutation) const;

	public:
		// Clones the first candidate into the first and all the new
		// ones. E.g. before = 3, after = 5, clones A in the three-candidate
		// election into A, D, and E in the five.
		clone_const(int numcands_before_in, int numcands_after_in);

		// Support cloning someone who is not A.
		clone_const(std::vector<int> after_as_before_in);
};