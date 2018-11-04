#ifndef _COMP_SCENARIO
#define _COMP_SCENARIO

#include "../../../../tools/tools.h"
#include "../../../../pairwise/abstract_matrix.h"

#include <set>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>

#include <iostream>
#include <iterator>

// This class represents a concrete Copeland scenario. The election method
// types we want to construct have different behavior for each Copeland
// scenario.

// Let a Copeland matrix c[A,B] be defined as
//		c[A, B] = 1		if A beats B pairwise
//		c[A, B] = 0		if B beats A pairwise
// (We don't handle ties as they occur comparatively rarely.)

// Let an election perspective be a pair consisting of an election and
// a candidate. 

// Two election perspectives have the same Copeland scenario if when the
// corresponding elections have their candidates relabeled so that the
// perspective's candidate is the first candidate, the two elections are
// associated with the same Copeland matrix.

// Two elections (not part of a perspective) have the same Copeland scenario
// if the two elections are associated with the same Copeland matrix.

// Formally speaking, we want to establish the equivalence classes of 
// election perspectives under the equivalence relation "has the same 
// Copeland scenario". To do so, we have to first create a class for
// handling Copeland scenarios, and finding such from elections. We need a
// function to relabel candidates and get the transformed Copeland scenario
// so we can determine equivalences between perspectives elsewhere.

// As a Copeland scenario can be represented in many ways, we also need
// functions for translating between the representations. (Note that the
// == and != operators do not implement the equivalence relation above, but 
// directly compares scenarios.)

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
		size_t number_of_candidates;

		// Gives the number of elements in a short form vector, given the
		// number of candidates.
		size_t num_short_form_elements(size_t numcands) const {
			return numcands * (numcands-1) / 2;
		}

		std::vector<bool> integer_to_short_form(const 
			uint64_t int_form, size_t numcands) const;

		uint64_t short_form_to_integer(const 
			std::vector<bool> & short_form) const;

		std::vector<bool> copeland_matrix_to_short_form(const
			std::vector<std::vector<bool> > & copeland_matrix) const;

		std::vector<std::vector<bool> > short_form_to_copeland_matrix(const
			std::vector<bool> & short_form, size_t numcands) const;

		std::vector<std::vector<bool> > condorcet_to_copeland_matrix(const
			abstract_condmat * condorcet_matrix) const;

		std::vector<bool> int_to_vbool(size_t numcands) const;

		// Produces a Copeland matrix where the candidates have been
		// reordered according to the order vector. See the implementation
		// for more information.
		std::vector<std::vector<bool> > permute_candidates(const 
			std::vector<std::vector<bool> > & copeland_matrix, const
			std::vector<int> & order) const;

	public:
		void set_numcands(size_t numcands) {
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

		// From a Condorcet matrix
		template<typename T> copeland_scenario(const T & condorcet_matrix) :
			copeland_scenario(condorcet_to_copeland_matrix(condorcet_matrix)) 
			{}

		copeland_scenario(const std::vector<bool> & short_form,
			size_t numcands) {

			scenario = short_form_to_integer(short_form);
			set_numcands(numcands);
		}

		copeland_scenario(const uint64_t scenario_in, size_t numcands) {
			scenario = scenario_in;
			set_numcands(numcands);
		}

		copeland_scenario(size_t numcands) : copeland_scenario(0, numcands) {}
		copeland_scenario(int numcands) : copeland_scenario(0, (size_t)numcands) {
			if (numcands < 0) {
				throw std::runtime_error("copeland_scenario ctor with negative numcands!");
			}
		}

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

#endif