#pragma once
// Relative criterion constraint generator for mono-add-top.

#include "../direct_relative_criterion.h"

class mono_add_top_const : public direct_relative_criterion_const {
	private:
		std::pair<int, std::vector<int> > get_monotonicity_indices(
			const std::vector<int> & permutation, int cand_to_raise) const;

	protected:
		int candidate_to_favor; // candidate who the added ballots rank top

		bool permissible_addition(const std::vector<int> & permutation) const {
			return permutation[0] == candidate_to_favor;
		}

	public:
		mono_add_top_const(int numcands_in, int candidate_to_favor_in) :
			direct_relative_criterion_const(numcands_in) {
				candidate_to_favor = candidate_to_favor_in;
			}

		// Default to helping A.
		mono_add_top_const(int numcands_in) : mono_add_top_const(
			numcands_in, 0) {}

		std::string name() const { return "Mono-add-top"; }
};