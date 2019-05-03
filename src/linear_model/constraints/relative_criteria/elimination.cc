#include "elimination.h"
#include <numeric>

size_t elimination_util_const::get_num_noneliminated(const std::vector<int> &
	elimination_spec_in) const {

	size_t count = 0;

	for (int cand: elimination_spec_in) {
		if (cand > -1) { ++count; }
	}

	return count;
}

cand_pairs elimination_util_const::get_proper_candidate_reordering(
	const std::vector<int> & elimination_spec_in) const {

	cand_pairs out;

	for (size_t i = 0; i < elimination_spec_in.size(); ++i) {
		// If the candidate is to be eliminated, say so, otherwise
		// add its before and after as an ordinary pair.
		if (elimination_spec_in[i] == -1) {
			out.set_pair(i, CP_NONEXISTENT);
		} else {
			out.set_pair(i, elimination_spec[i]);
		}
	}

	return out;
}

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

void elimination_util_const::set_elimination_spec(
	std::vector<int> & elimination_spec_in) {

	size_t should_be_before = elimination_spec_in.size(),
		should_be_after = get_num_noneliminated(elimination_spec_in);

	if (numcands_before != should_be_before ||
		numcands_after != should_be_after) {
		throw std::runtime_error("set_elimination_spec: invalid elimination"
			" spec!");
	}

	elimination_spec = elimination_spec_in;
	candidate_reordering = get_proper_candidate_reordering(
		elimination_spec);
}

elimination_util_const::elimination_util_const(size_t numcands_before_in,
	size_t numcands_after_in) :
	direct_relative_criterion_const(numcands_before_in, numcands_after_in) {

	numcands_before = numcands_before_in;
	numcands_after = numcands_after_in;
	std::vector<int> elimination_spec_in(numcands_before_in, -1);
	std::iota(elimination_spec_in.begin(),
		elimination_spec_in.begin()+numcands_after_in, 0);
	set_elimination_spec(elimination_spec_in);
}

elimination_util_const::elimination_util_const(
	std::vector<int> elimination_spec_in) :
	direct_relative_criterion_const(
		// Number of candidates before elimination: length of vector
			elimination_spec_in.size(),
		// Number of candidates after: number of nonzeroes.
			get_num_noneliminated(elimination_spec_in)
		){

	set_elimination_spec(elimination_spec_in);
}