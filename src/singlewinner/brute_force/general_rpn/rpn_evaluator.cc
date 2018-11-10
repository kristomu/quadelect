#include "rpn_evaluator.h"

size_t gen_custom_function::get_num_referential_atoms(
	size_t numcands) const {

	// There are n! permutations...
	// Then there are n^2 positional values
	// Then there are n * (n-1) pairwise values (diagonal removed)

	return (factorial(numcands) + numcands * numcands +
		numcands * (numcands-1));
}

atom_bundle gen_custom_function::get_atom_bundle(
	algo_t atom_encoding, size_t numcands) const {

	size_t num_referential_atoms = get_num_referential_atoms(numcands);
	atom_bundle out;

	// If it's below this number, it's a reference.
	if (atom_encoding < num_referential_atoms) {
		out.is_reference = true;

		// It's a direct reference
		if (atom_encoding < (algo_t)factorial(numcands)) {
			out.is_direct_reference = true;
			out.idx = atom_encoding;
			return(out);
		}

		atom_encoding -= factorial(numcands);

		// It's a positional reference
		if (atom_encoding < numcands * numcands) {
			out.is_positional_reference = true;
			out.place = atom_encoding % numcands;
			out.cand = atom_encoding / numcands;
			return(out);
		}

		atom_encoding -= numcands * numcands;

		// It's a pairwise reference
		if (atom_encoding < numcands * (numcands-1)) {
			out.is_pairwise_reference = true;

			out.challenger = atom_encoding % (numcands-1);
			out.incumbent = atom_encoding / (numcands-1);

			// This is a way of avoiding the diagonal. We don't want to 
			// clutter our search space with pairwise contests of the form 
			// A>A.
			if (out.challenger >= out.incumbent) {
				++out.challenger;
			}
			return(out);
		}

		// This shouldn't happen; if we get here, there's an error in
		// get_num_referential_atoms.
		assert (1 != 1);
	}

	out.function = (gen_custom_funct_atom)(atom_encoding - 
		num_referential_atoms);

	return(out);
}

// Point of CPU use. Allocating the vectors takes time.
// TODO: Consider optimization.

std::vector<atom_bundle> gen_custom_function::decode_algorithm(
	algo_t algorithm_encoding, size_t numcands) const {

	// The decoding is a little more complex than you'd expect, because we
	// have to allow for the first digit having value 0. To do so, we let
	// the first digit have one higher radix, and subtract one from it. But
	// because not all places have the same radix, we must then do it
	// most significant digit first style.

	if (algorithm_encoding == 0) { return (std::vector<atom_bundle>()); }

	// First count how many digits we have.

	size_t radix = get_num_referential_atoms(numcands) + TOTAL_NUM_CONST_ATOMS;

	int digits = 0;
	algo_t greatest_power = 1, divisor;

	while (algorithm_encoding >= greatest_power * (radix + 1)) {
		++digits;
		greatest_power *= radix;
	}

	std::vector<atom_bundle> out;
	out.reserve(digits); // or is it digits+1? TODO: Check later

	// Then get the first digit...
	algo_t digit = algorithm_encoding / greatest_power;
	algorithm_encoding -= digit * greatest_power;
	greatest_power /= radix;

	out.push_back(get_atom_bundle(digit-1, numcands));

	// and subsequent digits.
	for (divisor = greatest_power; divisor > 0; divisor /= radix) {
		digit = algorithm_encoding / divisor;
		algorithm_encoding -= digit * divisor;

		out.push_back(get_atom_bundle(digit, numcands));
	}

	return(out);
}


matrix_indices gen_custom_function::get_positional_matrix_indices(
	size_t numcands) const {

	// Let input[] be the input vector of counts for each permutation.

	// sum over z: input[positional_matrix_indices[x][y][z]] 
	// gives the number of voters who voted candidate x in yth place.

	// E.g. for 3 candidates: positional_matrix_indices[0][0] for
	// "first preference count for A" would be {0, 1}, since ABC + ACB =
	// number of voters ranking A first.

	// We do this the brute force way. We generate all possible permutations
	// and add their indices to the relevant positional_matrix_indices[x][y]
	// array.

	matrix_indices pos_indices(numcands);
	size_t cand, place, perm_count = 0;

	for (cand = 0; cand < numcands; ++cand) {
		pos_indices[cand].resize(numcands);
	}

	std::vector<int> ballot_permutation(numcands);
	std::iota(ballot_permutation.begin(), ballot_permutation.end(), 0);

	do {
		for (place = 0; place < numcands; ++place) {
			int cur_candidate = ballot_permutation[place];

			pos_indices[cur_candidate][place].push_back(perm_count);
		}
		++perm_count;
	} while (std::next_permutation(ballot_permutation.begin(), 
		ballot_permutation.end()));

	return(pos_indices);
}

bool gen_custom_function::does_a_beat_b(int a, int b, 
	const std::vector<int> & ballot_permutation) const {

	for (int v: ballot_permutation) {
		if (v == a) return(true);
		if (v == b) return(false);
	}

	throw std::runtime_error(
		"does A beat B: ballot permutation is incomplete!");
}

matrix_indices gen_custom_function::get_pairwise_matrix_indices(
	size_t numcands) const {

	// Let input[] be the input vector of counts for each permutation.

	// sum over z: input[pairwise_matrix_indices[x][y][z]] 
	// gives the number of voters who voted candidate x ahead of y.

	// As above, we do this the brute-force way, by generating every
	// permutation and then seeing which fits.

	matrix_indices pair_indices(numcands);
	size_t incumbent, challenger, perm_count = 0;

	std::vector<int> ballot_permutation(numcands);
	std::iota(ballot_permutation.begin(), ballot_permutation.end(), 0);

	for (incumbent = 0; incumbent < numcands; ++incumbent) {
		pair_indices[incumbent].resize(numcands);
	}

	do {
		for (incumbent = 0; incumbent < numcands; ++incumbent) {
			for (challenger = 0; challenger < numcands; ++challenger) {
				if (incumbent == challenger) continue;

				// This could be reduced to constant time by inverting the
				// permutation, but eh, can't be bothered.
				if (does_a_beat_b(incumbent, challenger, 
						ballot_permutation)) {
					pair_indices[incumbent][challenger].push_back(
						perm_count);
				}
			}
		}
		++perm_count;
	} while (std::next_permutation(ballot_permutation.begin(), 
		ballot_permutation.end()));

	return(pair_indices);
}

double gen_custom_function::linear_combination(const std::vector<int> & indices,
	const std::vector<double> & weights) const {

	double sum = 0;

	for (int idx : indices) {
		sum += weights[idx];
	}

	return(sum);
}

// XXX: We could make this quicker by offloading the linear combination
// calculation. I.e. one function takes in an election (v<d>) and outputs
// an expanded v<d>, then the main evaluate function accesses this
// expanded v<d>. E.g. fpA is expanded[n!+1], then the main evaluate can
// just access index n!+1 whenever it wants fpA. That would work as long as
// the number of candidate functions to test is much greater than the number
// of elections. But is it worth it? Go for the low-hanging fruit first.

double gen_custom_function::evaluate_ref(const atom_bundle & cur_alias, 
	const std::vector<double> & input_values, size_t numcands) const {

	// The first n! above this
	// refer to the ballot data, e.g. TOTAL_NUM_CONST_ATOMS in a three-
	// candidate function is ABC and TOTAL_NUM_CONST_ATOMS + 6 is CBA.
	// The next n^2 refer to the positional matrix, and the next n^2
	// to the pairwise data.

	// Is it a direct reference?
	if (cur_alias.is_direct_reference) {
		return input_values[cur_alias.idx];
	}

		// Is it a positional reference?
	if (cur_alias.is_positional_reference) {
		return linear_combination(positional_matrix_indices
			[cur_alias.cand][cur_alias.place], input_values);
	}

	// Is it a pairwise reference?

	if (cur_alias.is_pairwise_reference) {
		return linear_combination(pairwise_matrix_indices
			[cur_alias.incumbent][cur_alias.challenger], input_values);
	}

	throw std::runtime_error("Evaluate_ref: atom is inconsistent!");
}

// We're always generous to asymptotes here (see custom_function for what
// that means). It may be interesting to relax this later, but for now...

// A lot of the code is copied from custom_funct since the functions are the
// same.

double gen_custom_function::evaluate(std::vector<double> & stack,
	const atom_bundle & cur_raw_atom, 
	const std::vector<double> & input_values, size_t numcands) const {

	bool generous_to_asymptotes = true;

	// First check for references to ballot data or linear combinations of
	// such.

	if (cur_raw_atom.is_reference) {
		return evaluate_ref(cur_raw_atom, input_values, numcands);
	}

	// Check for functions and constants. Return the appropriate
	// value if cur_atom is a function or constant.

	gen_custom_funct_atom cur_atom = cur_raw_atom.function;

	// Constants:

	switch(cur_atom) {
		case VAL_IN_ALL: 
			return std::accumulate(input_values.begin(), 
				input_values.end(), 0.0);

		case VAL_ZERO: return(0);
		case VAL_ONE: return(1);
		case VAL_TWO: return(2);
		default: break;
	}

	// Unary and multi-value functions require at least one value on the stack.
	if (stack.empty()) {
		return(std::numeric_limits<double>::quiet_NaN());
		//throw runtime_error("stack error");
	}

	// All-stack functions that empty the whole stack must have at least one
	// value on the stack, and so go here or below.

	// Functions where applying the function once is a nop will go down by
	// binary functions. E.g. ALL_FUNC_PLUS is a vector addition operation,
	// and doing plus with just one element is a no operation.

	// ---

	// Then we have functions with a specified number of parameters, unary
	// ones first:

	double right_arg = *(stack.rbegin());
	if (isnan(right_arg)) return(right_arg); // NaNs propagate automatically
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

	// Binary functions require at least two values on the stack.
	if (stack.empty()) {
		return(std::numeric_limits<double>::quiet_NaN());
		//throw runtime_error("stack error");
	}

	double middle_arg = *(stack.rbegin());
	if (isnan(middle_arg)) return(middle_arg); // NaN propagate automatically
	stack.pop_back();

	// See above for why ALL_FUNC_PLUS is here.

	// Sums up every element on the stack.
	if (cur_atom == ALL_FUNC_PLUS) {
		double sum_stack = std::accumulate(stack.begin(), stack.end(), 0.0);
		stack.clear();
		return (sum_stack + middle_arg + right_arg);
	}

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
				return(std::numeric_limits<double>::quiet_NaN());
			} 
			if (right_arg == 0) {
				if (generous_to_asymptotes) {
					return (middle_arg/(right_arg+1e-9));
				}
				//return(std::numeric_limits<double>::quiet_NaN());
				return(INFINITY); // could also be -infty
			}
			return(middle_arg/right_arg);
		case BINARY_FUNC_MAX: return(std::max(middle_arg, right_arg));
		case BINARY_FUNC_MIN: return(std::min(middle_arg, right_arg));
		default: break;
	}

	// Error, should never happen. This happens if the atom is not caught
	// anywhere, which means I forgot to implement the code after adding
	// one.
	throw std::runtime_error("RPN token not matched anywhere!");
}

double gen_custom_function::evaluate(
	const std::vector<atom_bundle> & algorithm,
	const std::vector<double> & input_values, size_t numcands) const {

	algorithm_stack.clear();

	for(const atom_bundle & atom: algorithm) {
		double output = evaluate(algorithm_stack, atom, input_values, 
			numcands);

		if (isnan(output)) return(output);

		algorithm_stack.push_back(output);
	}

	// Don't permit more than one value to remain on the stack.
	if (algorithm_stack.size() != 1) {
		return(std::numeric_limits<double>::quiet_NaN());
	}

	// Otherwise, return the one value.
	return (*algorithm_stack.begin());
}

std::string gen_custom_function::get_atom_name(
	const atom_bundle & cur_atom, size_t numcands) const {

	if (cur_atom.is_positional_reference) {
		char cand_name = 'A' + cur_atom.cand;
		return "pos(" + itos(cur_atom.place) + "," + cand_name + ")";
	}

	if (cur_atom.is_pairwise_reference) {
		char incumbent_name = 'A' + cur_atom.incumbent;
		char challenger_name = 'A' + cur_atom.challenger;

		return std::string(1, incumbent_name) + ">" + challenger_name;
	}

	if (cur_atom.is_direct_reference) {
		return factoradic().kth_permutation(cur_atom.idx, numcands); 
	}

	// Okay, it's not a reference, so find out what kind of atom it is.
	assert(!cur_atom.is_reference);

	switch(cur_atom.function) {
		case VAL_IN_ALL: return "|V|";
		case VAL_ZERO: return "0";
		case VAL_ONE: return "1";
		case VAL_TWO: return "2";
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
		case BINARY_FUNC_MIN: return("MIN");
		case BINARY_FUNC_MAX: return("MAX");
		case ALL_FUNC_PLUS: return("v+");
		default: return("???");
	}
}

std::string gen_custom_function::to_string() const {

	bool is_first = true;

	std::string out = "";

	for (const atom_bundle & atom: current_algorithm) {
		if (!is_first) {
			out = out + " ";
		}
		is_first = false;
		out += get_atom_name(atom, number_candidates);
	}

	return out;
}

// This function returns false (and doesn't set anything) if evaluating the 
// algorithm on a standard input produces NaN, which is the error signal for
// the evaluator.

// XXX: This is a point of optimization. The initial evaluation takes
// a considerable amount of time when sifting for interesting algorithms.
// We could perhaps make the decoder keep track of the arity of the
// functions it encounters and return false if the stack will be empty
// too soon or have more than one value on it once the evaluation is done.
bool gen_custom_function::set_algorithm(algo_t algorithm_encoding) {

	std::vector<atom_bundle> proposed_algorithm = decode_algorithm(
		algorithm_encoding, number_candidates);

	// Now test this with a standard array (all zeroes) to see if we get a
	// NaN.

	std::vector<double> fake_input(factorial(number_candidates), 0);
	double result = evaluate(proposed_algorithm, fake_input, 
		number_candidates);

	if (isnan(result)) {
		return(false);
	}

	current_algorithm = proposed_algorithm;
	return(true);
}

void gen_custom_function::force_set_algorithm(algo_t algorithm_encoding) {	
	current_algorithm = decode_algorithm(algorithm_encoding, 
		number_candidates);
}

bool gen_custom_function::test_positional_linear_combination() {
	set_num_candidates(3);

	// The indices are ABC ACB BAC BCA CAB CBA
	// fpA should be T T F F F F, i.e. 0 and 1
	// fpB should be F F T T F F, i.e. 2 and 3
	// fpC should be F F F F T T, i.e. 4 and 5

	return positional_matrix_indices[0][0] == std::vector<int>({0, 1}) &&
		positional_matrix_indices[1][0] == std::vector<int>({2, 3}) &&
		positional_matrix_indices[2][0] == std::vector<int>({4, 5});
}

bool gen_custom_function::test_pairwise_linear_combination() {
	set_num_candidates(3);

	// The indices are ABC ACB BAC BCA CAB CBA
	// A>B should be 1 1 0 0 1 0, i.e. 0 1 4
	// A>C should be 1 1 1 0 0 0, i.e. 0 1 2
	// B>C should be 1 0 1 1 0 0, i.e. 0 2 3

	return pairwise_matrix_indices[0][1] == std::vector<int>({0, 1, 4}) &&
		pairwise_matrix_indices[0][2] == std::vector<int>({0, 1, 2}) &&
		pairwise_matrix_indices[1][2] == std::vector<int>({0, 2, 3});
}

bool gen_custom_function::test_pairwise_inference() {
	set_num_candidates(4);

	std::vector<double> input = {1,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,
		2,0,0,0,0,0};

	atom_bundle a_over_d;
	a_over_d.is_reference = true;
	a_over_d.is_pairwise_reference = true;
	a_over_d.incumbent = 0;
	a_over_d.challenger = 3;

	return evaluate_ref(a_over_d, input, 4) == 1;
}

bool gen_custom_function::test_pairwise_doubles() {
	set_num_candidates(3);

	std::vector<double> input = {1.1,0,0,0,0,0};

	atom_bundle a_over_b;
	a_over_b.is_reference = true;
	a_over_b.is_pairwise_reference = true;
	a_over_b.incumbent = 0;
	a_over_b.challenger = 1;

	return evaluate_ref(a_over_b, input, 3) == 1.1;
}