
#include "strong_um.h"
#include "../constraint_tools.h"

constraint strong_um::get_before_after_equality(
	std::string before_suffix, std::string after_suffix) const {

	constraint out;
	out.description = "before_after_equality__" + before_suffix + "__" + after_suffix;
	out.constraint_rel.type = LREL_EQ;
	out.constraint_rel.lhs = constraint_tools::permutations_to_relation_side(
		constraint_tools::all_permutations(numcands_before), "",
		before_suffix);
	out.constraint_rel.rhs = constraint_tools::permutations_to_relation_side(
		constraint_tools::all_permutations(numcands_after), "",
		after_suffix);

	return out;
}

constraint_set strong_um::majority_pairwise_beat(
	std::string before_suffix) const {

	// should perhaps be in pairwise? Do that if we need it elsewhere.
	lin_relation majority_beats;
	majority_beats.type = LREL_GE;
	majority_beats.lhs = constraint_tools::permutations_to_relation_side(
		constraint_tools::get_permutations_beating(0, manipulator,
			numcands_before), "", before_suffix);
	// greater than or equal to half the number of voters...
	majority_beats.rhs.weights.push_back(std::pair<std::string, double>(
		"v", 0.5));
	// plus margin.
	majority_beats.rhs.weights.push_back(std::pair<std::string, double>(
		"min_victory_margin", 1));

	constraint out_const;
	out_const.constraint_rel = majority_beats;
	out_const.description = (std::string)"strong_um_A_beats_" +
		(char)('A'+manipulator) + "_" + before_suffix;

	constraint_set out_set;
	out_set.add(out_const);

	return out_set;
}

constraint_set strong_um::no_compromising(std::string before_suffix,
	std::string after_suffix) const {

	constraint_set compromise_limits;

	// The strict < is not a bug. The case least_rank = numcands_after is
	// implemented by before_after_equality.
	for (size_t least_rank = 1; least_rank < numcands_after; ++least_rank) {
		// The sum of every ballot ranking the manipulator less than
		// least_rank ranks from bottom must be equal both before and after.
		// Se strong_um.h for explanation. This implements Chris Benahm's
		// revision of the criterion in [link].

		lin_relation compromise_limit;
		compromise_limit.type = LREL_GE;

		// TODO: Constraint tools should return size_t.
		for (std::vector<int> permutation:
			constraint_tools::all_permutations(numcands_after)) {

			bool ranks_b_below_at = false;

			for (size_t rank = 0; rank < least_rank; ++rank) {
				ranks_b_below_at |= (permutation[numcands_after - rank - 1]
					== (int)manipulator);
			}

			if (!ranks_b_below_at) { continue; }

			compromise_limit.lhs.weights.push_back(std::pair<std::string,
				double>(constraint_tools::permutation_to_str(permutation,
					after_suffix), 1));
			compromise_limit.rhs.weights.push_back(std::pair<std::string,
				double>(constraint_tools::permutation_to_str(permutation,
					before_suffix), 1));
		}
		if (compromise_limit.lhs.weights.empty()) { continue; }

		constraint compromise_limit_const;
		compromise_limit_const.description = "no_compromising_rank_" +
			itos(least_rank) + before_suffix + after_suffix;
		compromise_limit_const.constraint_rel = compromise_limit;
		compromise_limits.add(compromise_limit_const);
	}

	return(compromise_limits);
}

constraint_set strong_um::retain_honest_ballots(std::string before_suffix,
	std::string after_suffix) const {

	constraint_set retain_consts;

	for (std::vector<int> permutation:
		constraint_tools::all_permutations(numcands_after)) {

		// If it ranks A ahead of the manipulator candidate, then it's
		// a honest ballot and the after ballot count must be at least as
		// high.

		size_t i;

		for (i = 0; i < permutation.size() &&
			permutation[i] != 0 &&
			(size_t)permutation[i] != manipulator;
			++i) {
		}

		if (i == permutation.size() || (size_t)permutation[i] == manipulator) {
			continue;
		}

		constraint honest;
		honest.description = "um_honest_" +
			constraint_tools::permutation_to_str(permutation,
				before_suffix) + after_suffix;

		lin_relation honest_rel;
		honest_rel.type = LREL_LE;
		honest_rel.lhs.weights.push_back(std::pair<std::string,
			double>(constraint_tools::permutation_to_str(permutation,
				before_suffix), 1));
		honest_rel.rhs.weights.push_back(std::pair<std::string,
			double>(constraint_tools::permutation_to_str(permutation,
				after_suffix), 1));

		honest.constraint_rel = honest_rel;

		retain_consts.add(honest);
	}

	return retain_consts;
}

constraint_set strong_um::relative_constraints(std::string before_suffix,
	std::string after_suffix) const {

	constraint_set out_set;

	// The sum of before ballots should equal the sum of after ballots.
	out_set.add(get_before_after_equality(before_suffix, after_suffix));

	// A must beat B pairwise by a majority.
	out_set.add(majority_pairwise_beat(before_suffix));

	// Compromise strategy is not permitted.
	out_set.add(no_compromising(before_suffix, after_suffix));

	// Every after ballot that is not in the hands of the strategists
	// (i.e. every ballot that ranks A over the manipulator) must have
	// the after ballot value be greater than or equal to the before
	// ballot. That means that strategists may vote this way, but the
	// honest voters who voted that way must keep voting that way.
	out_set.add(retain_honest_ballots(before_suffix, after_suffix));

	return out_set;
}

strong_um::strong_um(size_t numcands_in, size_t manipulator_in) :
	relative_criterion_const(numcands_in, numcands_in) {

	assert(manipulator_in != 0);

	manipulator = manipulator_in;
	candidate_reordering = get_proper_candidate_reordering();
}