#include "constraint_set.h"
#include <stdexcept>
#include <set>

// Get all variable listed as part of a constraint but not set as
// a fixed parameter.

std::vector<std::string> constraint_set::get_free_variables(
	const std::vector<constraint> & relevant_constraints,
	const std::map<std::string, double> & fixed_params) const {

	std::set<std::string> parameters;

	for (const constraint & c: relevant_constraints) {
		for (const std::pair<std::string, double> & var :
			c.constraint_rel.lhs.weights) {
			if (fixed_params.find(var.first) ==
				fixed_params.end()) {
				parameters.insert(var.first);
			}
		}
		for (const std::pair<std::string, double> & var :
			c.constraint_rel.rhs.weights) {
			if (fixed_params.find(var.first) ==
				fixed_params.end()) {
				parameters.insert(var.first);
			}
		}
	}

	std::vector<std::string> out;
	std::copy(parameters.begin(), parameters.end(), std::back_inserter(
			out));

	return out;
}

void constraint_set::print_gmpl_program() {
	print_free_variables_gmpl();
	print_fixed_parameters_gmpl();
	print();
}

void constraint_set::print_free_variables_gmpl() const {
	std::vector<std::string> free_vars = get_free_variables(
			constraints, fixed_parameters);

	for (std::string var: free_vars) {
		std::cout << "var " << var << ";\n";
	}
}


void constraint_set::print_fixed_parameters_gmpl() const {
	for (std::pair<std::string, double> fixed : fixed_parameters) {
		std::cout << "param " << fixed.first << " := " << fixed.second
			<< ";\n";
	}
}

void constraint_set::add(const constraint_set & other) {
	// First add the constraints themselves.
	add(other.constraints);

	// Then add any fixed parameters. Throw an exception if we have a
	// collision, because otherwise it would be hard to reason about
	// what the fixed parameter gets set to.

	for (std::pair<std::string, double> param: other.fixed_parameters) {
		if (fixed_parameters.find(param.first) != fixed_parameters.end()) {
			if (fixed_parameters[param.first] != param.second) {
				throw std::logic_error("constraint_set::add : conflicting \
					fixed parameter values for " + param.first);
			}
		}
		fixed_parameters[param.first] = param.second;
	}
}