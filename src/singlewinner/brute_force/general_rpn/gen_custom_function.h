#ifndef _GRPN_EVAL
#define _GRPN_EVAL

// This class evaluates an algorithm for determing the score of a candidate
// in an n-candidate election. The algorithm is represented as an integer,
// which is decoded into a list of tokens (atoms) and evaluated RPN style.

// What we need are
// n! atoms for the number of voters for each permutation
// n^2 atoms for positional matrix aliases (first place votes for A, etc)
// n^2 atoms for pairwise matrix aliases (A>B etc)

// one atom for the number of voters in total
// 3 atoms for the constants 0, 1, 2,

// Some unary functions (log, exp, etc)
// Some binary functions (plus, minus, etc)

// Since we don't know how many n! will come out to, we need the functions
// to have the lower numbers and everything else be higher numbers. Although
// the positional and pairwise matrix aliases are, technically speaking,
// redundant (they're linear combinations of the n! variables), they
// constitute subsolutions that have been useful before. For instance,
// with four candidates, minmax can be written as A>B A>C A>D MIN MIN
// with pairwise aliases, but needs at least 72 atoms if we only have the
// ABCD...DCBA variables, as the pairwise variables alias to 12 of these
// plus 12 pluses each.

// We might need DAC/DSC values later, too.

// We could possibly refactor this quite a bit by moving most of the
// evaluation logic inside each atom. But again, eh. I'll do it if it's
// necessary.

#include <math.h>
#include <assert.h>

#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <stdexcept>

#include "../rpn/chaotic_functions.h"
#include "../../../tools/tools.h"
#include "../../../tools/factoradic.h"

#include <iostream>

enum gen_custom_funct_atom {
	VAL_IN_ALL = 0,	// num voters
	VAL_ZERO = 1,
	VAL_ONE = 2,
	VAL_TWO = 3,

	UNARY_FUNC_INFRMAP = 4,	// map (-inf,+inf) to (0,1)
	UNARY_FUNC_INFRMAPINV = 5,	// ditto the other way
	UNARY_FUNC_SQUARE = 6,
	UNARY_FUNC_SQRT = 7,
	UNARY_FUNC_LOG = 8,
	UNARY_FUNC_EXP = 9,
	UNARY_FUNC_NEG = 10,
	UNARY_FUNC_BLANCMANGE = 11,
	UNARY_FUNC_MINKOWSKIQ = 12,

	BINARY_FUNC_PLUS = 13,
	BINARY_FUNC_MINUS = 14,
	BINARY_FUNC_MUL = 15,
	BINARY_FUNC_DIVIDE = 16,
	BINARY_FUNC_MIN = 17,
	BINARY_FUNC_MAX = 18,

	ALL_FUNC_PLUS = 19,

	TOTAL_NUM_CONST_ATOMS = 20
};

// We unpack the integer into a list of these. An atom bundle can either
// be a reference to a positional variable (number of voters voting x at yth
// rank), to a pairwise variable (number of voters voting x above y), or a
// command/constant.

struct atom_bundle {
	bool is_reference = false;

	bool is_direct_reference = false;
	int idx;

	bool is_positional_reference = false;
	int cand, place;

	bool is_pairwise_reference = false;
	int incumbent, challenger;

	gen_custom_funct_atom function;
};

typedef std::vector<std::vector<std::vector<int> > > matrix_indices;
typedef unsigned long long algo_t;

class gen_custom_function {
	private:
		matrix_indices positional_matrix_indices;
		matrix_indices pairwise_matrix_indices;

		std::vector<atom_bundle> current_algorithm;
		algo_t current_algorithm_num; // For caching purposes
		mutable std::vector<double> algorithm_stack;

		size_t number_candidates;

		const double blancmange_order = 0.67;

		size_t get_num_referential_atoms(size_t numcands) const;

		atom_bundle get_atom_bundle(algo_t atom_encoding, size_t numcands) const;
		std::vector<atom_bundle> decode_algorithm(algo_t
			algorithm_encoding, size_t numcands) const;

		bool does_a_beat_b(int a, int b, const std::vector<int> &
			ballot_permutation) const;

		matrix_indices get_positional_matrix_indices(size_t numcands) const;
		matrix_indices get_pairwise_matrix_indices(size_t numcands) const;

		double linear_combination(const std::vector<int> & indices,
			const std::vector<double> & weights) const;

		double evaluate_ref(const atom_bundle & cur_alias,
			const std::vector<double> & input_values,
			size_t numcands) const;

		double evaluate(std::vector<double> & stack,
			const atom_bundle & cur_atom,
			const std::vector<double> & input_values, size_t numcands) const;

		double evaluate(const std::vector<atom_bundle> & algorithm,
			const std::vector<double> & input_values, size_t numcands) const;

		std::string get_atom_name(const atom_bundle & cur_atom,
			size_t numcands) const;

	public:
		double evaluate(const std::vector<double> & input_values) const {
			return evaluate(current_algorithm, input_values,
				number_candidates);
		}

		std::string to_string() const;

		bool set_algorithm(algo_t algorithm_encoding);
		void force_set_algorithm(algo_t algorithm_encoding);
		void set_num_candidates(size_t number_candidates_in) {
			number_candidates = number_candidates_in;
			positional_matrix_indices = get_positional_matrix_indices(
				number_candidates);
			pairwise_matrix_indices = get_pairwise_matrix_indices(
				number_candidates);
		}

		gen_custom_function(size_t number_candidates_in) {
			// Since force_set_algorithm checks if the algorithm to set is the
			// one we already have, valgrind will report a conditional jump
			// on uninitialized values if we don't set current_algorithm_num
			// to some value (not zero) before forcing algorithm zero.
			current_algorithm_num = 1;

			set_num_candidates(number_candidates_in);
			force_set_algorithm(0);
		}

		gen_custom_function(size_t number_candidates_in, algo_t algorithm) :
			gen_custom_function(number_candidates_in) {
				force_set_algorithm(algorithm);
		}

		bool test_positional_linear_combination();
		bool test_pairwise_linear_combination();
		bool test_pairwise_inference();
		bool test_pairwise_doubles();
		bool test() {
			return test_positional_linear_combination() &&
				test_pairwise_linear_combination() &&
				test_pairwise_inference()/* &&
				test_pairwise_doubles()*/;
		}
};

#endif