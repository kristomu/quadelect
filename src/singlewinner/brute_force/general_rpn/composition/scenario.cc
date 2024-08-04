#include "scenario.h"
#include "pairwise/matrix.h"
#include "pairwise/types.h"

std::vector<bool> copeland_scenario::integer_to_short_form(const
	uint64_t int_form, size_t numcands) const {

	std::vector<bool> short_form;

	uint64_t max_mask = 1ULL << (num_short_form_elements(numcands) - 1);

	for (uint64_t mask = max_mask; mask > 0; mask >>= 1) {
		short_form.push_back(int_form & mask);
	}

	return short_form;
}

uint64_t copeland_scenario::short_form_to_integer(const
	std::vector<bool> & short_form) const {

	uint64_t output = 0;

	for (bool x: short_form) {
		output <<= 1;
		if (x) {
			output++;
		}
	}

	return output;
}


std::vector<std::vector<bool> >
copeland_scenario::short_form_to_copeland_matrix(const
	std::vector<bool> & short_form, size_t numcands) const {

	std::vector<std::vector<bool> > copeland_matrix(numcands,
		std::vector<bool>(numcands, false));

	size_t scenario_ctr = 0;
	for (size_t i = 0; i < numcands; ++i) {
		for (size_t j = i+1; j < numcands; ++j) {
			copeland_matrix[i][j] = short_form[scenario_ctr];
			copeland_matrix[j][i] = !copeland_matrix[i][j];
			++scenario_ctr;
		}
	}

	return copeland_matrix;
}

std::vector<std::vector<bool> >
copeland_scenario::condorcet_to_copeland_matrix(const
	abstract_condmat * condorcet_matrix) const {

	size_t numcands = condorcet_matrix->get_num_candidates();

	std::vector<std::vector<bool> > copeland_matrix(numcands,
		std::vector<bool>(numcands, false));

	for (size_t i = 0; i < numcands; ++i) {
		for (size_t j = i+1; j < numcands; ++j) {
			if (condorcet_matrix->get_magnitude(i, j) ==
				condorcet_matrix->get_magnitude(j, i)) {
				// Perhaps return 0-candidate scenario instead?
				throw std::runtime_error(
					"Copeland_scenario: pairwise ties not supported!");
			}
			copeland_matrix[i][j] = (condorcet_matrix->get_magnitude(
						i, j) > condorcet_matrix->get_magnitude(j, i));
			copeland_matrix[j][i] = !copeland_matrix[i][j];
		}
	}

	return copeland_matrix;
}

std::vector<std::vector<bool> >
copeland_scenario::election_to_copeland_matrix(const
	election_t & election, size_t numcands) const {

	// TODO: Move to mutable.
	condmat condorcet_matrix(CM_PAIRWISE_OPP);

	condorcet_matrix.zeroize();
	condorcet_matrix.count_ballots(election, numcands);
	return condorcet_to_copeland_matrix(&condorcet_matrix);
}

std::vector<bool> copeland_scenario::copeland_matrix_to_short_form(const
	std::vector<std::vector<bool> > & copeland_matrix) const {

	size_t numcands = copeland_matrix.size();

	std::vector<bool> short_form;

	for (size_t i = 0; i < numcands; ++i) {
		for (size_t j = i+1; j < numcands; ++j) {
			short_form.push_back(copeland_matrix[i][j]);
		}
	}

	return short_form;
}

// Permute the Copeland matrix into what it would be if we relabeled the
// candidates in an election and then inferred the Copeland matrix from
// that election.

// Order is a vector containing the ballot index of the candidate who's
// to become the nth. For instance,
// order = {3, 2, 0, 1} means
// what will be the first candidate (A) in the output is D (#3) in the input
// second candidate (B) in the output is C (#2) in the input
// third candidate (C) in the output is A (#0) in the input,
// and fourth candidate (D) in the output is B (#1) in the input.

std::vector<std::vector<bool> > copeland_scenario::permute_candidates(const
	std::vector<std::vector<bool> > & copeland_matrix, const
	std::vector<int> & order) const {

	std::vector<std::vector<bool> > out_cm(copeland_matrix.size(),
		std::vector<bool>(copeland_matrix.size(), false));

	for (size_t i = 0; i < copeland_matrix.size(); ++i) {
		for (size_t j = 0; j < copeland_matrix.size(); ++j) {
			out_cm[i][j] = copeland_matrix[order[i]][order[j]];
		}
	}

	return out_cm;
}

std::string copeland_scenario::to_string() const {
	std::string outstr = itos(scenario) + "," + itos(number_of_candidates) +
		"/";

	for (bool x: integer_to_short_form(scenario, number_of_candidates)) {
		if (x) {
			outstr += "T";
		} else {
			outstr += "F";
		}
	}
	return outstr;
}

// Yeah, I know, unit tests...

/*int main() {
	copeland_scenario x(4);
	if (x.test()) {
		std::cout << "OK" << std::endl;
	}
}*/