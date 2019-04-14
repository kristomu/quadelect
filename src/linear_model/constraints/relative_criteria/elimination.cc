#include "elimination.h"
#include <numeric>

bool elimination_util_const::permissible_transition(
	const std::vector<int> & before_permutation,
	const std::vector<int> & after_permutation) const {

	// Permissible if, when we run the before permutation through the
	// spec, we get the after.

	std::vector<int> before_perm_after_elim;

	for (int cand: before_permutation) {
		if (elimination_spec[cand] != -1) {
			before_perm_after_elim.push_back(elimination_spec[cand]);
		}
	}

	return before_perm_after_elim == after_permutation;
}

elimination_util_const::elimination_util_const(size_t numcands_before_in,
	size_t numcands_after_in) :
	relative_criterion_const(numcands_before_in, numcands_after_in) {

	numcands_before = numcands_before_in;
	numcands_after = numcands_after_in;
	std::vector<int> elimination_spec_in(numcands_before_in, -1);
	std::iota(elimination_spec_in.begin(),
		elimination_spec_in.begin()+numcands_after_in, 0);
	elimination_spec = elimination_spec_in;
}

elimination_util_const::elimination_util_const(
	std::vector<int> elimination_spec_in) : 
	relative_criterion_const
		// Number of candidates before elimination: length of vector
		(elimination_spec_in.size(),
		// Number of candidates after: maximum element + 1 (due to zero
		// indexing).
			1 + *std::max_element(elimination_spec_in.begin(),
			elimination_spec_in.end())
		){

	elimination_spec = elimination_spec_in;
}