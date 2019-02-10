#include "numvoters.h"
#include "constraint_tools.h"

// Defines that the election corresponding to situation_suffix can at
// most have voters_var voters.

constraint_set voter_constraints::max_numvoters_definition(int numcands,
	std::string situation_suffix) const {

	constraint out;
	out.description = "numvoters_max_constraint_" + situation_suffix;
	out.constraint_rel.type = LREL_LE;
	out.constraint_rel.lhs = constraint_tools::permutations_to_relation_side(
		constraint_tools::all_permutations(numcands), "", situation_suffix);
	out.constraint_rel.rhs.weights.push_back(std::pair<std::string, double>(
		voters_var, 1));

	return out;
}


// Bound the number of voters to be maximum numvoters. Used when we want
// to e.g. optimize the number of voters.

constraint_set voter_constraints::max_numvoters_upper_bound(
	int maximum) const {

	constraint upper_bound_v;
	upper_bound_v.description = "upper_bound_v";
	upper_bound_v.constraint_rel.lhs.weights.push_back(
		std::pair<std::string, double>(voters_var, 1));
	upper_bound_v.constraint_rel.rhs.constant = maximum;
	upper_bound_v.constraint_rel.type = LREL_LE;

	return upper_bound_v;

}
