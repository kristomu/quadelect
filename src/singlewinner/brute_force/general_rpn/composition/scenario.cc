// This class represents a concrete Copeland scenario. The election method
// types we want to construct have different behavior for each Copeland
// scenario.

// Let a Copeland matrix c[A,B] be defined as
//		c[A, B] = 1		if A beats B pairwise
//		c[A, B] = 0		if B beats A pairwise
// (We don't handle ties as they occur comparatively rarely.)

// Two elections have the same Copeland scenario if they're associated with
// the same Copeland matrix, or if it's possible to permute the candidate
// names of one election to get the same Copeland matrix as the other 
// election.

// (Formally speaking, "has the same Copeland scenario" is an equivalence
// relation over elections with no pairwise ties, and we want to specify
// a gen_custom_function for each equivalence class in order to create an
// election method.)
//		TODO: This needs to be modified since we also have to handle
//		candidate rotation. E.g. to call the election in scenario 10,
//		we need to determine the score for every candidate from a starting
//		point of scenario 10 (from A's perspective).

#include "../../../../tools/tools.h"

#include <set>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>

#include <iostream>
#include <iterator>

// This class handles Copeland scenarios and goes from pairwise matrices
// to scenario booleans. Note that < does not implement the equivalence
// relation above, but directly distinguishes scenarios.

// We have three ways of representing a given scenario:
//		Long form, which is the Copeland matrix itself,

//		Short form, which collapses the lower diagonal matrix onto a linear
//		Boolean vector, e.g. the 4-candidate version lists the Copeland
//		elements A>B A>C A>D B>C B>D C>D in order,

//		Integer form, which is the integer corresponding to the short form
//		vector.

// This class uses integer form internally, so that incrementing and
// decrementing is easy. Note that this limits the number of candidates
// to around 11, as any more will lead to an overflow.

// (I think the increment/decrement thing will have to be handled in
// compositor.)

class copeland_scenario {
	private:
		uint64_t scenario;
		uint64_t max_scenario;
		int number_of_candidates;

		// Gives the number of elements in a short form vector, given the
		// number of candidates.
		int num_short_form_elements(int numcands) const {
			return numcands * (numcands-1) / 2;
		}

		std::vector<bool> integer_to_short_form(const 
			uint64_t int_form, int numcands) const;

		uint64_t short_form_to_integer(const 
			std::vector<bool> & short_form) const;

		std::vector<bool> copeland_matrix_to_short_form(const
			std::vector<std::vector<bool> > & copeland_matrix) const;

		std::vector<std::vector<bool> > short_form_to_copeland_matrix(const
			std::vector<bool> & short_form, int numcands) const;

		std::vector<bool> int_to_vbool(int numcands) const;

		// Produces a Copeland matrix where the candidates have been
		// reordered according to the order vector. See the implementation
		// for more information.
		std::vector<std::vector<bool> > permute_candidates(const 
			std::vector<std::vector<bool> > & copeland_matrix, const
			std::vector<int> & order) const;

	public:
		void set_numcands(int numcands) {
			number_of_candidates = numcands;
			max_scenario = (1 << num_short_form_elements(numcands)) - 1;
			scenario &= max_scenario;
		}

		bool operator<(const copeland_scenario & other) const {
			if (number_of_candidates != other.number_of_candidates) {
				return number_of_candidates < other.number_of_candidates;
			}

			return scenario < other.scenario;
		}

		std::vector<bool> get_short_form() const {
			return integer_to_short_form(scenario, number_of_candidates);
		}

		std::vector<std::vector<bool> > get_copeland_matrix() const {
			return short_form_to_copeland_matrix(get_short_form(), 
				number_of_candidates);
		}

		int get_numcands() const { return number_of_candidates; }

		std::string to_string() const;

		copeland_scenario(const std::vector<std::vector<bool> > & 
			copeland_matrix) {

			scenario = short_form_to_integer(
				copeland_matrix_to_short_form(copeland_matrix));
			set_numcands(copeland_matrix.size());
		}

		copeland_scenario(const std::vector<bool> & short_form,
			int numcands) {

			scenario = short_form_to_integer(short_form);
			set_numcands(numcands);
		}

		copeland_scenario(const uint64_t scenario_in, int numcands) {
			scenario = scenario_in;
			set_numcands(numcands);
		}

		copeland_scenario(int numcands) : copeland_scenario(0, numcands) {}

		// Default to number of candidates = 1 for maps and such.
		// Such a default is useless for any real work, so remember to
		// use a proper constructor.
		copeland_scenario() : copeland_scenario(1) {}

		// Not exactly quick, but it gets the job done. Fix later if we
		// need a speedup.
		void permute_candidates(const std::vector<int> & permutation) {
			std::vector<bool> short_form = integer_to_short_form(scenario, 
				number_of_candidates);

			std::vector<std::vector<bool> > copeland_matrix =
				short_form_to_copeland_matrix(short_form, number_of_candidates);

			std::vector<bool> new_short_form = copeland_matrix_to_short_form(
				permute_candidates(copeland_matrix, permutation));

			scenario = short_form_to_integer(new_short_form);
		}

		// For enumeration purposes
		copeland_scenario & operator++() {
			// With wraparound.
			scenario = (scenario + 1) & max_scenario;
			return *this;
		}

		copeland_scenario & operator--() {
			// With wraparound.
			scenario = (scenario - 1) & max_scenario;
			return *this;
		}

		copeland_scenario operator++(int throwaway) {
			copeland_scenario unincremented(*this);
			operator++();
			return unincremented;
		}

		copeland_scenario operator--(int throwaway) {
			copeland_scenario unincremented(*this);
			operator--();
			return unincremented;
		}

		bool operator==(const copeland_scenario & other) const {
			return scenario == other.scenario && 
				number_of_candidates == other.number_of_candidates;
		}

		bool operator!=(const copeland_scenario & other) const {
			return scenario != other.scenario ||
				number_of_candidates != other.number_of_candidates;
		}

		bool test() {
			bool a = (short_form_to_integer(integer_to_short_form(59, 4)) 
				== 59);

			std::vector<int> perm(number_of_candidates, 0);
			std::iota(perm.begin(), perm.end(), 0);

			set_numcands(4);
			scenario = 59;
			permute_candidates(perm);

			bool b = (scenario == 59);

			return a && b;
		}
};

std::vector<bool> copeland_scenario::integer_to_short_form(const 
	uint64_t int_form, int numcands) const {

	std::vector<bool> short_form;

	uint64_t max_mask = 1 << num_short_form_elements(numcands) - 1;

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
	std::vector<bool> & short_form, int numcands) const {

	std::vector<std::vector<bool> > copeland_matrix(numcands, 
		std::vector<bool>(numcands, false));

	int scenario_ctr = 0;
	for (int i = 0; i < numcands; ++i) {
		for (int j = i+1; j < numcands; ++j) {
			copeland_matrix[i][j] = short_form[scenario_ctr];
			copeland_matrix[j][i] = !copeland_matrix[i][j];
			++scenario_ctr;
		}
	}

	return copeland_matrix;
}

std::vector<bool> copeland_scenario::copeland_matrix_to_short_form(const
	std::vector<std::vector<bool> > & copeland_matrix) const{
	
	int numcands = copeland_matrix.size();

	std::vector<bool> short_form;

	for (int i = 0; i < numcands; ++i) {
		for (int j = i+1; j < numcands; ++j) {
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

	for (int i = 0; i < copeland_matrix.size(); ++i) {
		for (int j = 0; j < copeland_matrix.size(); ++j) {
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

/*main() {
	copeland_scenario x(4);
	if (x.test()) {
		std::cout << "OK" << std::endl;
	}
}*/