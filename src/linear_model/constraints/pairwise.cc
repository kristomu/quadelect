#include "pairwise.h"
#include "constraint_tools.h"

// Creates inequality representing A beating B pairwise (or the other way
// around). If A beats B, it's [a beats b permutations] - margin >=
// [b beats a permutations], otherwise the other way around. margin is used
// to turn > into >=, since the polytopes don't support strict inequality.

lin_relation pairwise_constraints::get_beat_equation(bool a_beats_b, int a, 
	int b, std::string suffix, int numcands) const {

	lin_relation out;
	out.type = LREL_GE;

	// Assume A beats B
	out.lhs = constraint_tools::permutations_to_relation_side(
		constraint_tools::get_permutations_beating(
		a, b, numcands), "", suffix);
	out.rhs = constraint_tools::permutations_to_relation_side(
		constraint_tools::get_permutations_beating(
		b, a, numcands), "", suffix);

	// If B beats A, swap the two sides.
	if (!a_beats_b) {
		std::swap(out.lhs, out.rhs);
	}

	// Subtract an epsilon from the lhs.
	out.lhs.weights.push_back(std::pair<std::string, double>(margin, -1));

	return out;
}

constraint pairwise_constraints::beat_constraint(bool a_beats_b, int a,
	int b, std::string ballot_suffix, std::string description_suffix,
	int numcands) const {

	lin_relation beats = get_beat_equation(a_beats_b, a, b, ballot_suffix,
		numcands);

	constraint beat_const;
	beat_const.constraint_rel = beats;

	if (a_beats_b) {
		beat_const.description = description_suffix + "_" + (char)('A'+a)
			+ "_beat_" + (char)('A'+b);
	} else {
		beat_const.description = description_suffix + "_" + (char)('A'+b)
			+ "_beat_" + (char)('A'+a);
	}

	return beat_const;
}

constraint_set pairwise_constraints::beat_constraints(
	const std::vector<bool> & short_form_copeland, std::string suffix,
	int numcands) const {

	int linear_count = 0;

	std::vector<constraint> constraints;

	for (int a = 0; a < numcands; ++a) {
		for (int b = a+1; b < numcands; ++b) {
			bool a_beats_b = short_form_copeland[linear_count++];

			constraints.push_back(beat_constraint(a_beats_b, a, b,
				suffix, suffix, numcands));
		}
	}

	return constraint_set(constraints);
}
