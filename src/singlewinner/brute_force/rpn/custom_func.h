// This is the "ext_custfunc" from my dirty experimental code elsewhere. I
// saw no reason to keep basic custfunct.

// Custom function: A custom function constructed from basic functions and
// input variables by RPN.

// ---------------------------------------

// TO NOTE for later: When we construct a function, because it is bottom up,
// there are three possible ways it can go:
// 1. Too few outputs (stack underrun error). Then this function can't be 
//		used.
// 2. One output: this function can be used.
// 3. Multiple outputs: this function can be used.

// This then suggests that we can combine smaller functions that we know do 
// work into larger functions or pieces of larger functions. More specifically,
// let the number of outputs be some integer p, and the number of stack inputs
// needed to be some integer q.
// If q > 0, can be suffixed to any function whose p >= q.
// If p == 1, can be used directly
// If p >= 1, can be prefixed as above.
// ... and then we can recursively build very large functions.

#ifndef _RPN_CUSTOM_FUNC
#define _RPN_CUSTOM_FUNC

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <math.h>

using namespace std;

// TODO? Pare this down somewhat. Max and min can be used to
// directly construct if-then clauses.
enum custom_funct_atom {
	VAL_IN_ABC = 0,
	VAL_IN_ACB = 1,
	VAL_IN_BAC = 2,
	VAL_IN_BCA = 3,
	VAL_IN_CAB = 4,
	VAL_IN_CBA = 5,
	VAL_IN_FPA = 6,		// ABC+ACB
	VAL_IN_FPB = 7,		// BAC+BCA
	VAL_IN_FPC = 8,		// CAB+CBA
	VAL_IN_AbB = 9,		// ABC + ACB + CAB 0 1 4
	VAL_IN_AbC = 36,	// ABC + ACB + BAC 0 1 2
	VAL_IN_BbA = 37,	// BAC + BCA + CBA 2 3 5
	VAL_IN_BbC = 10,	// BCA + BAC + ABC 2 3 0 
	VAL_IN_CbA = 11,	// CAB + CBA + BCA 3 4 5
	VAL_IN_CbB = 13,	// CAB + CBA + ACB 3 4 1
	VAL_IN_ALL = 12,	// num voters
	VAL_ZERO = 14,
	VAL_ONE = 15,
	VAL_TWO = 16,
	UNARY_FUNC_INFRMAP = 17,	// map (-inf,+inf) to (0,1)
	UNARY_FUNC_INFRMAPINV = 18,	// ditto the other way
	UNARY_FUNC_SQUARE = 19,
	UNARY_FUNC_SQRT = 20,
	UNARY_FUNC_LOG = 21,
	UNARY_FUNC_EXP = 22,
	UNARY_FUNC_NEG = 23,
	UNARY_FUNC_BLANCMANGE = 38,
	UNARY_FUNC_MINKOWSKIQ = 39,
	BINARY_FUNC_PLUS = 24,
	BINARY_FUNC_MINUS = 25,
	BINARY_FUNC_MUL = 26,
	BINARY_FUNC_DIVIDE = 27,
	BINARY_FUNC_MIN = 28,
	BINARY_FUNC_MAX = 29,
	// Comparators
	// If we have x>y, we can get y<x by inverting
	BINARY_FUNC_LT = 30,
	BINARY_FUNC_LEQ = 31,
	BINARY_FUNC_EQ = 32,
	BINARY_FUNC_AND = 33,
	BINARY_FUNC_OR = 34,
	TERNARY_FUNC_IF = 35, // if a then push b otherwise push c
	TOTAL_NUM_ATOMS = 40
// For future: while/wend unary functions: if the last token in the wend is
// true, start from where the while started. Kinda tricky, though, so later.
// (Would of course have a max iter number counter)
};

class custom_function {
	private:
		vector<custom_funct_atom> our_function;
		mutable vector<double> funct_stack;
		mutable vector<double> test_results;

		const double blancmange_order = 0.67;

		// All negative values are false and non-negative values are true.
		// This lets neg work as not in a sense (except for a singular point).
		bool is_true(const double & in) const { return (in >= 0); }
		int make_bool(const bool & in) const {
			if (in) { return(0); }
			return(-1);
		}

		void decode_function(unsigned long long function_number,
			vector<custom_funct_atom> & output) const;
		double evaluate(vector<double> & stack, 
			const custom_funct_atom & cur_atom, 
			const vector<double> & input_values) const;
		double evaluate_function(const vector<custom_funct_atom> & function,
			const vector<double> & input_values, 
			bool reject_large_stack) const;
		bool update_suitability(const custom_function & funct_to_test, 
			const vector<vector<double> > & test_in_vectors,
			map<vector<double>, unsigned long long> & results_already_seen) const;
		bool update_ordinal_suitability(const custom_function & 
			funct_to_test, const vector<vector<double> > & test_in_vectors,
			map<vector<bool>, unsigned long long> & results_already_seen) const;
		
		string atom_to_word(const custom_funct_atom in) const;
		unsigned long long our_value;

	public:
		void set_id(unsigned long long function_number);
		custom_function(unsigned long long function_number);
		custom_function();
		double evaluate(const vector<double> & input_values, 
			bool reject_large_stack) const;

		// Determine if the function works on the test vectors.
		bool update_suitability(const vector<vector<double> > & test_in_vectors,
			map<vector<double>, unsigned long long > & results_already_seen) const {
			return(update_suitability(*this, test_in_vectors, 
				results_already_seen));
		}

		bool update_ordinal_suitability(const vector<vector<double> > & test_in_vectors,
			map<vector<bool>, unsigned long long > & results_already_seen) const {
			return(update_ordinal_suitability(*this, test_in_vectors, 
				results_already_seen));
		}

		vector<string> get_atom_printout() const;
		unsigned long long get_value() const { return(our_value); }
};

#endif
