#pragma once

#include "../direct_relative_criterion.h"

// Relative criterion constraint generator for independence of clones.
// The generator imposes a clone constraint linking before and after,
// so that the given candidate in before is cloned into two candidates in
// after.

// TODO: Somehow find out how to specify the three types of independence of
// clones:
//		- Teaming: Cloning A shouldn't help A
//		- Vote-splitting: Cloning A shouldn't harm A
//		- Crowding: Cloning B shouldn't help/harm A

class clone_const : public direct_relative_criterion_const {
	private:
		// Check that after_as_before is valid. (TODO, use this.)
		bool check_after_as_before(
			std::vector<int> after_as_before_in) const;

		void set_after_as_before(
			std::vector<int> after_as_before_in) const;

	protected:
		bool permissible_transition(
			const std::vector<int> & before_permutation,
			const std::vector<int> & after_permutation) const;

		virtual bool is_valid_numcands_combination() const {
			return numcands_before < numcands_after; }

	public:
		// Clones the first candidate into the first and all the new
		// ones. E.g. before = 3, after = 5, clones A in the three-candidate
		// election into A, D, and E in the five.
		clone_const(size_t numcands_before_in, size_t numcands_after_in);

		// For crowding, we need to clone someone who isn't A.
		clone_const(size_t numcands_before_in, size_t numcands_after_in,
			size_t candidate_to_clone);

		// Support cloning someone who is not A, more generally.
		// TODO: See if there's a way to clone so that either A' or B'
		// is 55,4.
		clone_const(std::vector<int> after_as_before_in);

		// If A wins before and loses after cloning A: vote splitting.
		bool no_harm() const { return true; }

		// If A loses before and wins after cloning: teaming.
		bool no_help() const { return true; }

		std::string name() const { return "Clone Independence"; }
};