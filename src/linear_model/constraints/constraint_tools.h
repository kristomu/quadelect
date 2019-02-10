#pragma once

#include <vector>
#include <string>

#include "../lin_relation/lin_relation.h"

// I don't really know where to put these functions, so we get a catch-all
// class like "tools". The reason is that most constraints need to deal with
// the individual ballot types, which are permutations of candidate orders.
// Thus most of the constraints (no matter whether they're monotonicity
// classes or what have you) need a way of generating permutations and
// turning them into strings for variable names.

// Hidden inside here there's probably an alternate ballot structure (as
// vectors of permutations) along with functions to manipulate them, enumerate
// all permutations and so on. It might be an idea at some point to separate
// out these, but for now, I'll just hack it together like this.

class constraint_tools {
	public:

		// does A precede B in the permutation?
		static bool does_beat(int A, int B,
			const std::vector<int> & permutation);

		static std::vector<std::vector<int> > all_permutations(
			int numcands);

		// Get the permutations where A beats B
		static std::vector<std::vector<int> > get_permutations_beating(
			int A, int B, int numcands);

		static std::string permutation_to_str(
			const std::vector<int> & permutation, std::string suffix);

		static relation_side permutations_to_relation_side(
			const std::vector<std::vector<int> > & permutations,
			std::string prefix, std::string suffix);
};