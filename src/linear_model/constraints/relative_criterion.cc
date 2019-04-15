#include "relative_criterion.h"
#include "constraint_tools.h"

#include <algorithm>
#include <numeric>

void relative_criterion_const::add_addition_removal_terms(
	relation_side & add_where, std::string after_suffix,
	const std::vector<int> & cur_permutation) const {

	std::string perm_string = constraint_tools::permutation_to_str(
		cur_permutation, after_suffix);

	if (permissible_addition(cur_permutation)) {
		std::string increase_count_name = "inc_" + perm_string;
		add_where.weights.push_back(std::pair<std::string, double>(
			increase_count_name, 1));
	}

	if (permissible_deletion(cur_permutation)) {
		std::string decrease_count_name = "dec_" + perm_string;
		add_where.weights.push_back(
			std::pair<std::string, double>(decrease_count_name, -1));
	}
}

constraint relative_criterion_const::get_before_after_equality(
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

	// If we're adding or deleting anything, these terms have to be on the
	// left hand side, otherwise we can't add or delete anything at all.
	// So fix that. TODO? Relabel this function?

	for (std::vector<int> perm : constraint_tools::all_permutations(
		numcands_after)) {
		add_addition_removal_terms(out.constraint_rel.lhs, after_suffix,
			perm);
	}

	return out;
}


// Transitions: produces a map of, for each after-ballot, what before-
// ballots contribute to its count. (E.g. for mono-raise: after ABC
// is a sum of before ABC, before BAC, and before BCA.)

std::map<std::vector<int>, std::vector<std::vector<int> > >
	relative_criterion_const::get_after_from_before_transitions() const {

	std::vector<int> after_perm(numcands_after);
	std::iota(after_perm.begin(), after_perm.end(), 0);

	std::map<std::vector<int>, std::vector<std::vector<int> > >
		transitions;

	do {
		std::vector<int> before_perm(numcands_before);
		std::iota(before_perm.begin(), before_perm.end(), 0);

		do {
			if (permissible_transition(before_perm, after_perm)) {
				transitions[after_perm].push_back(before_perm);
			}
		} while (std::next_permutation(before_perm.begin(),
			before_perm.end()));
	} while (std::next_permutation(after_perm.begin(),
		after_perm.end()));

	return transitions;
}

// Reverses the map produced by the above function in case we need a map
// of before-ballots contributing to a particular after-ballot.
std::map<std::vector<int>, std::vector<std::vector<int> > >
	relative_criterion_const::reverse_map(const std::map<std::vector<int>,
	std::vector<std::vector<int> > > & in) const {

	std::map<std::vector<int>, std::vector<std::vector<int> > > out;

	for (const std::pair<std::vector<int>, std::vector<std::vector<int> > > &
		kv : in) {

		for (const std::vector<int> & in_sec_entry: kv.second) {
			out[in_sec_entry].push_back(kv.first);
		}
	}

	return out;
}

// These constraints link before-ballots (the election prior to
// modification) to the after-ballots (the election after modification,
// e.g. raising A). There are two of them: after-constraints that set
// the after ballots as a sum of before ballots (e.g.
// ABCa = ABCb + BACb + BCAb), and before-constraints that set the before
// ballots as a sum of after-ballots, thus preserving the voting count.
// (The latter may be unnecessary; find out later.)

// We now add addition and deletion terms to the after constraint, i.e.
// the constraints that define the after ballots, if the relative criterion
// says we should add or delete anything. We don't need to add these terms
// to the before constraints because the before constraints connect before
// variables to the before->after transition counts, not directly to the
// after variables, and thus addition and deletion is invisible there.

constraint_set relative_criterion_const::get_after_constraints(const
	std::map<std::vector<int>, std::vector<std::vector<int> > > &
	after_from_before_transitions, std::string before_suffix,
	std::string after_suffix) const {

	std::vector<constraint> after_consts;

	// For each permutation, if that permutation is linked to any before-
	// ballots (or insertions or deletions), create an equation linking
	// the two. If not, create an equation setting that permutation to
	// zero.

	for (std::vector<int> permutation:
		constraint_tools::all_permutations(numcands_after)) {

		std::string to_after = constraint_tools::permutation_to_str(
			permutation, after_suffix);

		constraint out;
		out.description = "permitted_transitions_after_" + to_after;

		lin_relation after_equals_befores;
		after_equals_befores.type = LREL_EQ;
		after_equals_befores.lhs.weights.push_back(std::pair<std::string,
			double>(to_after, 1));

		// If the current after-permutation is linked to anything, specify
		// it.
		if (after_from_before_transitions.find(permutation) !=
			after_from_before_transitions.end()) {
			after_equals_befores.rhs =
				constraint_tools::permutations_to_relation_side(
				after_from_before_transitions.find(permutation)->second, "",
				before_suffix + "_" + to_after);
		}

		// Add additions and deletions if the current permutation is one
		// that we want to add to or remove from.

		add_addition_removal_terms(after_equals_befores.rhs,
			after_suffix, permutation);

		out.constraint_rel = after_equals_befores;

		after_consts.push_back(out);
	}

	return after_consts;
}

// Is this necessary?
constraint_set relative_criterion_const::get_before_constraints(const
	std::map<std::vector<int>, std::vector<std::vector<int> > > &
	before_to_after_transitions, std::string before_suffix,
	std::string after_suffix) const {

	std::vector<constraint> before_consts;

	for (const std::pair<std::vector<int>, std::vector<std::vector<int> > > &
		before_and_after : before_to_after_transitions) {

		std::string from_before = constraint_tools::permutation_to_str(
			before_and_after.first, before_suffix);

		constraint out;
		out.description = "conservation_of_voters_" + from_before + "__" +
			after_suffix;

		lin_relation before_equals_afters;
		before_equals_afters.type = LREL_EQ;
		before_equals_afters.lhs.weights.push_back(std::pair<std::string,
			double>(from_before, 1));
		before_equals_afters.rhs =
			constraint_tools::permutations_to_relation_side(
			before_and_after.second, from_before + "_", after_suffix);
		out.constraint_rel = before_equals_afters;

		before_consts.push_back(out);
	}

	return before_consts;
}

constraint_set relative_criterion_const::relative_constraints(
	std::string before_suffix, std::string after_suffix) const {

	// First do a test for validity. (I wanted to put it in the ctor, but
	// that proved too much of a mess with inheritance.)

	if (!is_valid_numcands_combination()) {
		throw std::runtime_error("relative criterion: invalid "
		 "numcands combination: " + itos(numcands_before) + ", " +
		 itos(numcands_after));
	}

	// Okay, good to go.

	constraint_set out_set;

	// Before and after constraint
	out_set.add(get_before_after_equality(before_suffix, after_suffix));

	std::map<std::vector<int>, std::vector<std::vector<int> > >
		after_before = get_after_from_before_transitions();

	out_set.add(get_after_constraints(after_before, before_suffix,
		after_suffix));

	out_set.add(get_before_constraints(reverse_map(after_before),
		before_suffix, after_suffix));

	return out_set;
}

// The default is that all the candidates are preserved.
std::vector<int> relative_criterion_const::get_default_after_as_before() const {

	std::vector<int> out(numcands_before);
	std::iota(out.begin(), out.end(), 0);

	return out;
}