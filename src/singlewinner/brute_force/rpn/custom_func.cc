#include "custom_func.h"
#include "chaotic_functions.h"

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
		case VAL_IN_AbC: return("A>C");
		case VAL_IN_BbA: return("B>A");
		case VAL_IN_BbC: return("B>C");
		case VAL_IN_CbA: return("C>A");
		case VAL_IN_CbB: return("C>B");
		case VAL_IN_ALL: return("ALL");
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
		case UNARY_FUNC_BLANCMANGE: return("BLANCMANGE");
		case UNARY_FUNC_MINKOWSKIQ: return("MINK?");
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
// If "generous_to_asymptotes" is on, we replace division by zero and log
// zero with the relevant operator applied to very small values, so that
// something that might be of use, like LOG(CBA) won't fail just because
// CBA is 0.
// (However, in practice, this makes very strange methods come in first
// at the strategy evaluation.)
double custom_function::evaluate(vector<double> & stack, 
	const custom_funct_atom & cur_atom, 
	const vector<double> & input_values,
	bool generous_to_asymptotes) const {

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
		case VAL_IN_AbC: return(input_values[0]+input_values[1]+input_values[2]);
		case VAL_IN_BbA: return(input_values[2]+input_values[3]+input_values[5]);
		case VAL_IN_BbC: return(input_values[2]+input_values[3]+input_values[0]);
		case VAL_IN_CbA: return(input_values[3]+input_values[4]+input_values[5]);
		case VAL_IN_CbB: return(input_values[3]+input_values[4]+input_values[1]);
		case VAL_IN_ALL: return(input_values[0]+input_values[1]+input_values[2]+input_values[3]+input_values[4]+input_values[5]);
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

	double right_arg = *(stack.rbegin());
	if (isnan(right_arg)) return(right_arg); // NaN propagate automatically
	stack.pop_back();

	switch(cur_atom) {
		case UNARY_FUNC_INFRMAP: return (1/(1 + exp(-right_arg)));
		case UNARY_FUNC_INFRMAPINV: return(log(-right_arg/(right_arg-1)));
		case UNARY_FUNC_SQUARE: return(right_arg*right_arg);
		case UNARY_FUNC_SQRT: return(sqrt(right_arg));
		case UNARY_FUNC_LOG: 
			if (right_arg == 0) {
				if (generous_to_asymptotes) {
					// intend limit towards 0 so that 0 log 0 = 0, e.g	
					return (-1e9);		
				}
				return(-INFINITY);
			}
			return(log(right_arg));
		case UNARY_FUNC_EXP: return(exp(right_arg));
		case UNARY_FUNC_NEG: return(-right_arg);
		case UNARY_FUNC_BLANCMANGE: return(blancmange(blancmange_order,
			right_arg));
		case UNARY_FUNC_MINKOWSKIQ: return(minkowski_q(right_arg));
		default: break;
	}

	// Binary
	if (stack.empty()) {
		return(numeric_limits<double>::quiet_NaN());
		//throw runtime_error("stack error");
	}

	double middle_arg = *(stack.rbegin());
	if (isnan(middle_arg)) return(middle_arg); // NaN propagate automatically
	stack.pop_back();

	// NOTE: The stack has most recently pushed arguments to the right.
	// This means that if we want, say "fpA fpC -" to resolve to 
	// "fpA - fpC", as is intuitive, and as dc does it, we need to do
	// middle_arg - right_arg, not the other way around.

	switch(cur_atom) {
		case BINARY_FUNC_PLUS: return(middle_arg + right_arg);
		case BINARY_FUNC_MINUS: return(middle_arg - right_arg);
		case BINARY_FUNC_MUL: return(middle_arg * right_arg);
		case BINARY_FUNC_DIVIDE:
			if (!finite(middle_arg) && finite(right_arg) && right_arg != 0) {
				return(middle_arg);
			}
			// lim x->inf 3/x = 0
			if (finite(middle_arg) && !finite(right_arg)) {
				return(0);
			}
			// Perhaps we should let x/inf = 0? And inf/x = inf,
			// except when x = 0, in which case it's undefined.
			// inf/inf is also similarly undefined.
			if (!finite(middle_arg) && !finite(right_arg)) {
				return(numeric_limits<double>::quiet_NaN());
			} 
			if (right_arg == 0) {
				if (generous_to_asymptotes) {
					return (middle_arg/(right_arg+1e-9));
				}
				//return(numeric_limits<double>::quiet_NaN());
				return(INFINITY); // could also be -infty
			}
			return(middle_arg/right_arg);
		case BINARY_FUNC_MAX: return(max(middle_arg, right_arg));
		case BINARY_FUNC_MIN: return(min(middle_arg, right_arg));

		case BINARY_FUNC_LT: return(make_bool(middle_arg < right_arg));
		case BINARY_FUNC_LEQ: return(make_bool(middle_arg <= right_arg));
		case BINARY_FUNC_EQ: return(make_bool(middle_arg == right_arg));
		case BINARY_FUNC_AND: return(is_true(middle_arg) & is_true(right_arg));
		case BINARY_FUNC_OR: return(is_true(middle_arg) | is_true(right_arg));
		default: break;
	}

	if (stack.empty()) {
		return(numeric_limits<double>::quiet_NaN());
	}

	double left_arg = *(stack.rbegin());
	if (isnan(left_arg)) return(left_arg); // NaN propagate automatically
	stack.pop_back();

	switch(cur_atom) {
		case TERNARY_FUNC_IF: 
			if (is_true(left_arg)) {
				return(middle_arg);
			} else {
				return(right_arg);
			}
			break;
		default: break;
	}

	// Error, should never happen. This happens if the atom is not caught
	// anywhere, which means I forgot to implement the code after adding
	// one.
	throw runtime_error("RPN token not matched anywhere!");

	return(0);
}

// if reject_large_stack is true, will throw a runtime error if the
// stack is larger than size 1 (because then the function could have
// been pared down).
// I tried exceptions, but it's too slow. We return NaN on error.
double custom_function::evaluate_function(
	const vector<custom_funct_atom> & function,
	const vector<double> & input_values, bool reject_large_stack,
	bool generous_to_asymptotes) const {

	double result;

	//assert(funct_stack.empty());	// wouldn't want anything to carry over

	for (vector<custom_funct_atom>::const_iterator pos = function.begin(); 
		pos != function.end(); ++pos) {
		result = evaluate(funct_stack, *pos, input_values, 
			generous_to_asymptotes);
		if (isnan(result)) {
			// propagate error
			return(numeric_limits<double>::quiet_NaN());
		} else {
			funct_stack.push_back(result);
		}
	}

	// stack is empty
	if (funct_stack.empty()) {
		return(numeric_limits<double>::quiet_NaN());
	}

	// multiple outputs for function
	if (reject_large_stack && *funct_stack.begin() != *funct_stack.rbegin()) {
		return(numeric_limits<double>::quiet_NaN());
	}

	return(*funct_stack.rbegin());
}

void custom_function::set_id(unsigned long long function_number) {
	decode_function(function_number, our_function);
	our_value = function_number;
}

custom_function::custom_function(unsigned long long function_number,
	bool generous_in) {

	set_asymptote_generous(generous_in);
	set_id(function_number);
}

custom_function::custom_function(bool generous_in) {
	set_asymptote_generous(generous_in);
	set_id(0);
}

double custom_function::evaluate(const vector<double> & input_values,
	bool reject_large_stack, bool generous_to_asymptotes) const {
	funct_stack.clear();
	return(evaluate_function(our_function, input_values, 
		reject_large_stack, generous_to_asymptotes));
}

double custom_function::evaluate(const vector<double> & input_values,
	bool reject_large_stack) const {
	funct_stack.clear();
	return(evaluate_function(our_function, input_values, 
		reject_large_stack, is_generous_to_asymptotes));
}


// Runs the function on the test vectors specified. Returns true if it
// passes and returns different results on at least one of them, otherwise
// false.

bool custom_function::update_suitability(const custom_function & funct_to_test, 
	const vector<vector<double> > & test_in_vectors,
	map<vector<double>, unsigned long long> & results_already_seen) const {
	
	test_results.resize(test_in_vectors.size());
	size_t i;

	//std::cout << "Cardinal suitability" << std::endl;

	for (i = 0; i < test_in_vectors.size(); ++i) {
		/*copy(test_in_vectors[i].begin(), test_in_vectors[i].end(),
			ostream_iterator<double>(cout, " "));
		cout << " => ";*/
		test_results[i] = funct_to_test.evaluate(test_in_vectors[i], 
			true);
		//cout << test_results[i] << endl;
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
			// cout << our_value << ": cardinally shadowed by " << results_already_seen.find(test_results)->second << endl;
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
// Or we could use a Bloomier filter.
bool custom_function::update_ordinal_suitability(const custom_function & 
	funct_to_test, const vector<vector<double> > & test_in_vectors,
	map<vector<bool>, unsigned long long> & results_already_seen) const {

	vector<double> test_results;
	vector<bool> results_ordinal;

	size_t num_vectors = test_in_vectors.size();
	test_results.resize(num_vectors);
	results_ordinal.reserve(num_vectors*num_vectors);

	// The point of using tolerance is to disqualify functions that rely
	// on double precision roundoff errors to work. However, we want to
	// be more principled and only disqualify functions that don't meet our
	// criteria (even if the functions that do meet our criteria may use
	// weird ways of satisfying them), so this should be removed at some
	// point and be replaced by checking for things like the resolvability
	// criterion.
	double tolerance = 1e-9;

	size_t i, j;

	for (i = 0; i < num_vectors; ++i) {
		test_results[i] = funct_to_test.evaluate(test_in_vectors[i], 
			true);

		if (isnan(test_results[i])) {
			return(false);
		}
	}

	for (i = 0; i < num_vectors; ++i) {
		for (j = 0; j < num_vectors; ++j) {
			double difference = test_results[i]-test_results[j];
	
			results_ordinal.push_back(
				fabs(difference) > tolerance && difference < 0);
		}
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
