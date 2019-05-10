#pragma once

// Attempt at doing strong UM with fewer variables.
// Check if it's quicker, then clean up if it is.

// TODO: implement the newer version where the strategists are not
// permitted to raise B. Something like

// sum [everything with B last, after] >= sum [everything with B last, before]
// sum [everything with B last or next to, after] >= sum [everything with
//		B last or next to last, before]
// and so on.

// The first term ensures that no B-last ballot can be turned into a B-next
// to-last ballot. The second term ensures that no ballot with B in either
// last or next to last place can be turned into one with B in third to last,
// and so on.

#include "../relative_criterion.h"

class strong_um : public relative_criterion_const {
	private:
		size_t manipulator;

		cand_pairs get_proper_candidate_reordering() const {
			cand_pairs out;
			out.set_pair(manipulator, manipulator);
			return out;
		}

		constraint get_before_after_equality(std::string before_suffix,
			std::string after_suffix) const;

		constraint_set retain_honest_ballots(std::string before_suffix,
			std::string after_suffix) const;

		constraint_set majority_pairwise_beat(std::string before_suffix)
			const;

	protected:
		constraint_set relative_constraints(std::string before_suffix,
			std::string after_suffix) const;

	public:
		strong_um(size_t numcands_in, size_t manipulator_in);
		strong_um(size_t numcands) : strong_um(numcands, 1) {}

		std::string name() const { return "Strong Unmanipulable Majority"; }
};
