#pragma once

#include "../direct_relative_criterion.h"

// Relative criterion constraint generator for independence of clones.
// The generator imposes a clone constraint linking before and after,
// so that the given candidate in before is cloned into two candidates in
// after.

// The three types of independence of clones are:
//		- Teaming: Cloning A shouldn't help A
//		- Vote-splitting: Cloning A shouldn't harm A
//		- Crowding: Cloning B shouldn't help/harm A

// These are implemented by different after_as_before choices. The after_
// as before array is set so that after_as_before[x] gives the candidate
// index of (before) what is now candidate x. For instance, [0, 0, 1, 2]
// clones candidate A and so its no-harm tests against vote-splitting, and
// its no-help tests against teaming. [0, 1, 1, 2] clones candidate B and
// thus tests against crowding.

// Due to the way the relative criteria are set up, to cover every cloning
// from k to k+1 candidates, we need to create clone criteria for every
// possible after_as_before with after_as_before[0] = 0. At least that's
// what I think until I can prove otherwise (Which I should probably try to
// do at some point).

// However, in conjunction with ISDA, only "candidate to clone" and
// "candidate to clone into" need be specified. This because ISDA does its
// own permutations.

// Possible proof sketch: we're looking at every other candidate as B. So
// 0 1 1 2 from the perspective of 2 is no different from 0 2 2 1 from the
// perspective of 1. (Is that the case?, Not necessarily, but there's
// always a permutation that makes B into 1.)

class clone_const : public direct_relative_criterion_const {
	private:
		// This is the native format, and makes checking for permissible
		// transitions very easy.
		std::vector<int> after_as_before;

		cand_pairs get_proper_candidate_reordering(
			const std::vector<int> & after_as_before_in) const;

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

		clone_const(size_t numcands_in) :
			clone_const(numcands_in, numcands_in+1) {}

		clone_const(size_t numcands_before_in, size_t numcands_after_in,
			size_t candidate_to_clone, size_t candidate_to_clone_into);

		// Potentially clones into multiple.
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