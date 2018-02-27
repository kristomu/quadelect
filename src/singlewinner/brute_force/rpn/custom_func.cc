#include "custom_func.h"

#include <limits>
#include <iterator>

#include <assert.h>

// Turning the stack into a vector might also help with speed because
// reserving makes reallocation much faster.

void custom_function::decode_function(
		unsigned long long function_number,
		vector<custom_funct_atom> & output) const {
	
	if (function_number == 0) {
		return;
	}

	output.resize(0);

	while (function_number > 0) {
		unsigned long long curatom = function_number % TOTAL_NUM_ATOMS;
		output.push_back((custom_funct_atom)curatom);
		function_number /= TOTAL_NUM_ATOMS;
	}

	return;
}

string custom_function::atom_to_word(const custom_funct_atom in) const {

	// we should also have had modulo here, although I could only
	// conceivably see it being used in some sort of IRVesque system.
	switch(in) {
		case VAL_IN_ABC: return("ABC");
		case VAL_IN_ACB: return("ACB");
		case VAL_IN_BAC: return("BAC");
		case VAL_IN_BCA: return("BCA");
		case VAL_IN_CAB: return("CAB");
		case VAL_IN_CBA: return("CBA");
		case VAL_IN_FPA: return("fpA");
		case VAL_IN_FPB: return("fpB");
		case VAL_IN_FPC: return("fpC");
		case VAL_IN_AbB: return("A>B");
		case VAL_IN_BbC: return("B>C");
		case VAL_IN_CbA: return("C>A");
		case VAL_IN_ALL: return("ALL");
		case VAL_IN_DQ: return("DQ");
		case VAL_ZERO: return("0");
		case VAL_ONE: return("1");
		case VAL_TWO: return("2");
		case UNARY_FUNC_INFRMAP: return("INFRMAP");
		case UNARY_FUNC_INFRMAPINV: return("INFRMAP^-1");
		case UNARY_FUNC_SQUARE: return("SQUARE");
		case UNARY_FUNC_SQRT: return("SQRT");
		case UNARY_FUNC_LOG: return("LOG");
		case UNARY_FUNC_EXP: return("EXP");
		case UNARY_FUNC_NEG: return("NEG");
		case BINARY_FUNC_PLUS: return("+");
		case BINARY_FUNC_MINUS: return("-");
		case BINARY_FUNC_MUL: return("*");
		case BINARY_FUNC_DIVIDE: return("/");
		case BINARY_FUNC_MAX: return("MAX");
		case BINARY_FUNC_MIN: return("MIN");
		case BINARY_FUNC_LT: return("<");
		case BINARY_FUNC_LEQ: return("<=");
		case BINARY_FUNC_EQ: return("==");
		case BINARY_FUNC_AND: return("&&");
		case BINARY_FUNC_OR: return("||");
		case TERNARY_FUNC_IF: return("IF");
		default: return("???");
	}
}

// Returns NaN if it's not doable.
// (I used exceptions, but that was too slow.)
// Next performance improvement would be to make the stack fixed in size
// and just go up and down it instead of allocating list values. But eh.
double custom_function::evaluate(vector<double> & stack, 
	const custom_funct_atom & cur_atom, 
	const vector<double> & input_values) const {

	switch(cur_atom) {
		case VAL_IN_ABC: return(input_values[0]);
		case VAL_IN_ACB: return(input_values[1]);
		case VAL_IN_BAC: return(input_values[2]);
		case VAL_IN_BCA: return(input_values[3]);
		case VAL_IN_CAB: return(input_values[4]);
		case VAL_IN_CBA: return(input_values[5]);
		case VAL_IN_FPA: return(input_values[0] + input_values[1]);
		case VAL_IN_FPB: return(input_values[2] + input_values[3]);
		case VAL_IN_FPC: return(input_values[4] + input_values[5]);
		case VAL_IN_AbB: return(input_values[0]+input_values[1]+input_values[4]);
		case VAL_IN_BbC: return(input_values[2]+input_values[3]+input_values[0]);
		case VAL_IN_CbA: return(input_values[3]+input_values[4]+input_values[5]);
		case VAL_IN_ALL: return(input_values[0]+input_values[1]+input_values[2]+input_values[3]+input_values[4]+input_values[5]);
		case VAL_IN_DQ: return((input_values[0]+input_values[1]+input_values[2]+input_values[3]+input_values[4]+input_values[5])/4.0);
		case VAL_ZERO: return(0);
		case VAL_ONE: return(1);
		case VAL_TWO: return(2);
		default: break;
	}

	// Unary functions go here.
	if (stack.empty()) {
		return(numeric_limits<double>::quiet_NaN());
		//throw runtime_error("stack error");
	}

	double first_arg = *(stack.rbegin());
	stack.pop_back();

	switch(cur_atom) {
		case UNARY_FUNC_INFRMAP: return (1/(1 + exp(-first_arg)));
		case UNARY_FUNC_INFRMAPINV: return(log(-first_arg/(first_arg-1)));
		case UNARY_FUNC_SQUARE: return(first_arg*first_arg);
		case UNARY_FUNC_SQRT: return(sqrt(first_arg));
		case UNARY_FUNC_LOG: 
			if (first_arg == 0)
				return (-1e9);		// intend limit towards 0 so that 0 log 0 = 0, e.g
			return(log(first_arg));
		case UNARY_FUNC_EXP: return(exp(first_arg));
		case UNARY_FUNC_NEG: return(-first_arg);
		default: break;
	}

	// Binary
	if (stack.empty()) {
		return(numeric_limits<double>::quiet_NaN());
		//throw runtime_error("stack error");
	}

	double sec_arg = *(stack.rbegin());
	stack.pop_back();

	switch(cur_atom) {
		case BINARY_FUNC_PLUS: return(first_arg + sec_arg);
		case BINARY_FUNC_MINUS: return(first_arg - sec_arg);
		case BINARY_FUNC_MUL: return(first_arg * sec_arg);
		case BINARY_FUNC_DIVIDE:
			if (!finite(first_arg) || !finite(sec_arg)) {
				return(numeric_limits<double>::quiet_NaN());
			} 
			if (sec_arg == 0) {
				return (first_arg/(sec_arg+1e-9));
			}
			return(first_arg/sec_arg);
		case BINARY_FUNC_MAX: return(max(first_arg, sec_arg));
		case BINARY_FUNC_MIN: return(min(first_arg, sec_arg));

		case BINARY_FUNC_LT: return(make_bool(first_arg < sec_arg));
		case BINARY_FUNC_LEQ: return(make_bool(first_arg <= sec_arg));
		case BINARY_FUNC_AND: return(is_true(first_arg) & is_true(sec_arg));
		case BINARY_FUNC_OR: return(is_true(first_arg) | is_true(sec_arg));
		default: break;
	}

	if (stack.empty()) {
		return(numeric_limits<double>::quiet_NaN());
	}

	double third_arg = *(stack.rbegin());
	stack.pop_back();

	switch(cur_atom) {
		case TERNARY_FUNC_IF: 
			if (is_true(first_arg)) {
				return(sec_arg);
			} else {
				return(third_arg);
			}
			break;
		default: break;
	}

	// Error, should never happen. This happens if the atom is not caught
	// anywhere, which means I forgot to implement the code after adding
	// one.
	assert(1 != 1);

	return(0);
}

// if reject_large_stack is true, will throw a runtime error if the
// stack is larger than size 1 (because then the function could have
// been pared down).
// I tried exceptions, but it's too slow. We return NaN on error.
double custom_function::evaluate_function(
	const vector<custom_funct_atom> & function,
	const vector<double> & input_values, bool reject_large_stack) const {

	double result;

	//assert(funct_stack.empty());	// wouldn't want anything to carry over

	for (vector<custom_funct_atom>::const_iterator pos = function.begin(); 
		pos != function.end(); ++pos) {
		result = evaluate(funct_stack, *pos, input_values);
		if (isnan(result)) {
			// propagate error
			return(numeric_limits<double>::quiet_NaN());
		} else {
			funct_stack.push_back(result);
		}
	}

	if (funct_stack.empty()) {
		// throw(runtime_error("stack is empty"));
		return(numeric_limits<double>::quiet_NaN());
	}

	if (reject_large_stack && *funct_stack.begin() != *funct_stack.rbegin()) {
		return(numeric_limits<double>::quiet_NaN());
		//throw(runtime_error("multiple outputs for function!"));
	}

	return(*funct_stack.rbegin());
}

void custom_function::set_id(unsigned long long function_number) {
	decode_function(function_number, our_function);
	our_value = function_number;
}

custom_function::custom_function(unsigned long long function_number) {
	set_id(function_number);
}

custom_function::custom_function() {
	set_id(0);
}

double custom_function::evaluate(const vector<double> & input_values,
	bool reject_large_stack) const {
	funct_stack.clear();
	return(evaluate_function(our_function, input_values, 
		reject_large_stack));
}

// Runs the function on the test vectors specified. Returns true if it
// passes and returns different results on at least one of them, otherwise
// false.

bool custom_function::update_suitability(const custom_function & funct_to_test, 
	const vector<vector<double> > & test_in_vectors,
	map<vector<double>, unsigned long long> & results_already_seen) const {
	
	test_results.resize(test_in_vectors.size());
	size_t i;

	for (i = 0; i < test_in_vectors.size(); ++i) {
		test_results[i] = funct_to_test.evaluate(test_in_vectors[i], 
			true);
		if (isnan(test_results[i])) {
			return(false); // error during evaluation
		}
		//cout << results[results.size()-1] << "\t";
	}

	bool something_differs = false;
	for (i = 0; i < test_results.size()-1 && !something_differs; ++i)
		something_differs |= (test_results[i] != test_results[i+1]);

	if (something_differs) {
		if (results_already_seen.find(test_results) != results_already_seen.end()) {
			// Ew.
			//cout << our_value << ": cardinally shadowed by " << results_already_seen.find(test_results)->second << endl;
			return(false);
		}
		results_already_seen[test_results] = funct_to_test.get_value();
	}

	return(something_differs);
	
}

// Now this is the limiting factor (for memory).
// What we could do if we'd like Even More Functions is to set up a
// locally sensitive hash map (say by first k ordinals). If the function
// ends up matching any of these, run it through every function inside. The LSH
// should filter out most matches and then we only need to check a few if we do
// have a match by hash.
bool custom_function::update_ordinal_suitability(const custom_function & 
	funct_to_test, const vector<vector<double> > & test_in_vectors,
	map<vector<bool>, unsigned long long> & results_already_seen) const {

	vector<double> results_cardinal;
	vector<bool> results_ordinal;

	int num_vectors = test_in_vectors.size();
	results_cardinal.resize(num_vectors);
	results_ordinal.reserve(num_vectors*num_vectors);

	double eps = 1152921504606846976; // e.g.

	int i, j;

	for (i = 0; i < num_vectors; ++i) {
		results_cardinal[i] = round(
			funct_to_test.evaluate(test_in_vectors[i], 
			true) * eps)/eps;
		/*cout << results_cardinal[i] << "\t";
		copy(test_in_vectors[i].begin(), test_in_vectors[i].end(), ostream_iterator<double>(cout, " "));
		cout << endl;*/
	}

	for (i = 0; i < num_vectors; ++i) {
		for (j = 0; j < num_vectors; ++j) {
			double difference = results_cardinal[i]-results_cardinal[j];

			results_ordinal.push_back(difference < 0);

			/*if (difference<0) {
				cout << "T";
			} else {
				cout << "F";
			}*/
		}
		//cout << endl;
	}

	//cout << endl;

	if (results_already_seen.find(results_ordinal) != 
		results_already_seen.end()) {
		//cout << our_value << ": ordinally shadowed by " << results_already_seen.find(results_ordinal)->second << endl;
		return(false);
	} else {
		results_already_seen[results_ordinal] = funct_to_test.get_value();
	}

	return(true);
}

vector<string> custom_function::get_atom_printout() const {
	vector<string> printable_funct;

	for (vector<custom_funct_atom>::const_iterator pos = our_function.begin();
		pos != our_function.end(); ++pos) {
		printable_funct.push_back(atom_to_word(*pos));
	}

	return (printable_funct);
}