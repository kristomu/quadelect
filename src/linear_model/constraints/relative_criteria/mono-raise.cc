#include "mono-raise.h"

// The first item is the position of cand_to_raise in the permutation.
// The second item is the position of every other candidate if we pretend
//	cand_to_raise is not in the permutation.
std::pair<int, std::vector<int> >
mono_raise_const::get_monotonicity_indices(
	const std::vector<int> & permutation, int cand_to_raise) const {

	size_t numcands = permutation.size(), count_not_ctr = 0;

	std::vector<int> out_others(numcands, -1);
	int first = -1;

	for (size_t i = 0; i < permutation.size(); ++i) {
		if (permutation[i] == cand_to_raise) {
			first = i;
		} else {
			out_others[permutation[i]] = count_not_ctr++;
		}
	}
	return std::pair<int, std::vector<int> >(first, out_others);
}

// Is it permitted to transform a before-ballot into this after-ballot?
// E.g. for mono-raise (implemented here), the relative position of every
// candidate but A must be the same, and A must not be lower (closer to the
// end) after compared to before.

bool mono_raise_const::permissible_transition(
	const std::vector<int> & before_permutation,
	const std::vector<int> & after_permutation) const {

	if (before_permutation.size() != after_permutation.size()) {
		return false;
	}

	std::pair<int, std::vector<int> > before_indices =
		get_monotonicity_indices(before_permutation, 0), after_indices =
			get_monotonicity_indices(after_permutation, 0);

	return after_indices.first <= before_indices.first &&
		after_indices.second == before_indices.second;

}