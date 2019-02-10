#include "constraint_polytope.h"
#include "constraint_tools.h"
#include <iostream>

int constraint_polytope::add_new_variables(
	const constraint_set & fixed_param_ref, const relation_side & in) {

	int num_new_variables = 0;

	// So that I don't have to type so much when checking for fixed
	// parameters.
	std::map<std::string, double> fixed_parameters = fixed_param_ref.
		get_fixed_parameters();

	// Add every variable we haven't seen before that isn't a fixed
	// variable.

	for (const std::pair<std::string, double> & var: in.weights) {
		if (variable_names_idx.find(var.first) == variable_names_idx.
			end() && fixed_parameters.find(var.first) == fixed_parameters.
			end()) {

			size_t next_idx = variable_names_idx.size();

			variable_names_idx[var.first] = next_idx;
			++num_new_variables;
		}
	}

	return num_new_variables;
}

void constraint_polytope::add_relation(
	const constraint_set & fixed_param_ref, lin_relation in) {

	reduction_stale = true;

	std::map<std::string, double> fixed_parameters = fixed_param_ref.
		get_fixed_parameters();

	// If it's a greater-than-equal relation, flip the lhs and rhs so it's
	// a <= instead.

	if (in.type == LREL_GE) {
		std::swap(in.lhs, in.rhs);
		in.type = LREL_LE;
	}

	int num_new_variables = 0, num_old_variables = C.cols();

	// Add new variables and count how many.

	num_new_variables = add_new_variables(fixed_param_ref, in.lhs) +
		add_new_variables(fixed_param_ref, in.rhs);

	int num_variables = num_old_variables + num_new_variables;

	// The vector gets initialized to zero. Note that this is a column
	// vector so we need to take the transpose when we add it into the
	// matrix afterwards.
	Eigen::VectorXd this_constraint_in_matrix = Eigen::VectorXd::Zero(
		num_variables);

	double rhs_constant = in.rhs.constant, lhs_constant = in.lhs.constant;

	// Turn "ABC + ACB + CAB + 3 <= CBA - 2 * ACB + 10" into
	// [1 -1 0 0 1 -1] * [ABC ACB BAC BCA CAB CBA] <= 7

	for (const std::pair<std::string, double> & var: in.lhs.weights) {
		// If it's a fixed variable, just add it to the constant
		// element.
		if (fixed_parameters.find(var.first) != fixed_parameters.end()) {
			lhs_constant += var.second * fixed_parameters[var.first];
		} else {
			int this_idx = variable_names_idx[var.first];
			this_constraint_in_matrix[this_idx] += var.second;
		}
	}

	for (const std::pair<std::string, double> & var: in.rhs.weights) {
		// If it's a fixed variable, just add it to the constant
		// element.
		if (fixed_parameters.find(var.first) != fixed_parameters.end()) {
			rhs_constant += var.second * fixed_parameters[var.first];
		} else {
			int this_idx = variable_names_idx[var.first];
			this_constraint_in_matrix[this_idx] -= var.second;
		}
	}

	double this_objective_element = rhs_constant - lhs_constant;

	// Extend both C and F to contain all the new variables, even if
	// they don't use any of those new variables.
	C.conservativeResize(C.rows(), num_variables);
	C.block(0, num_old_variables, C.rows(), num_new_variables) =
		Eigen::MatrixXd::Zero(C.rows(), num_new_variables);
	F.conservativeResize(F.rows(), num_variables);
	F.block(0, num_old_variables, F.rows(), num_new_variables) =
		Eigen::MatrixXd::Zero(F.rows(), num_new_variables);

	// If it's an inequality, extend C and b; otherwise extend F and g.
	if (in.type == LREL_LE) {
		C.conservativeResize(C.rows()+1, Eigen::NoChange);
		C.block(C.rows()-1, 0, 1, num_variables) = this_constraint_in_matrix.
			transpose();
		d.conservativeResize(d.rows()+1);
		d[d.rows()-1] = this_objective_element;
	} else {
		assert(in.type == LREL_EQ);
		F.conservativeResize(F.rows()+1, Eigen::NoChange);
		F.block(F.rows()-1, 0, 1, num_variables) = this_constraint_in_matrix.
			transpose();
		g.conservativeResize(g.rows()+1);
		g[g.rows()-1] = this_objective_element;
	}
}

void constraint_polytope::add_relations(const constraint_set & in) {
	for (constraint x: in.constraints) {
		add_relation(in, x.constraint_rel);
	}
}

// Since add_relation always expands the matrices to have as many
// columns as there are variables, we can use their number of columns
// to get the number of variables. Note that we could also have used F.
int constraint_polytope::get_num_variables() const {
 	return C.cols();
 }

int constraint_polytope::get_variable_index(
	std::string variable_name) const {

	if (variable_names_idx.find(variable_name) == variable_names_idx.end()) {
		throw std::logic_error("get_variable_index: variable " +
			variable_name + " does not exist!");
	}

	return (variable_names_idx.find(variable_name)->second);
}

std::vector<int> constraint_polytope::get_variable_indices(
	const std::vector<std::string> & variable_prefixes,
	const std::string suffix) const {

	std::vector<int> out(variable_prefixes.size());

	for (size_t i = 0; i < variable_prefixes.size(); ++i) {
		out[i] = get_variable_index(variable_prefixes[i] + suffix);
	}

	return out;
}

std::vector<int> constraint_polytope::get_all_permutations_indices(
	int numcands, const std::string suffix) const {

	std::vector<int> out;

	for (std::vector<int> permutation : constraint_tools::all_permutations(
		numcands)) {

		out.push_back(get_variable_index(
			constraint_tools::permutation_to_str(permutation, suffix)));
	}

	return out;
}