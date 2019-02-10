// A set of constraints in vector form, with fixed parameter assignments and
// associated functions for easy printing.

// Fixed parameters override variable names with the value they're set to.
// When printing to GMPL/AMPL, this is done by first declaring a param
// to be some value, and then printing the constraint with the symbolic
// name of that param, so print() won't replace variable names for the
// fixed parameters with the value of that parameter. print_gmpl() will
// print the fixed parameters as param; and the others as var;.

#pragma once
#include "constraint.h"
#include <map>
#include <vector>
#include <iterator>

class constraint_set {
	private:
		// e.g. fixed_parameters["eps"] = 1
		std::map<std::string, double> fixed_parameters;

		// Printing functions.
		std::vector<std::string> get_free_variables(
			const std::vector<constraint> & relevant_constraints,
			const std::map<std::string, double> & fixed_params) const;

		void print_free_variables_gmpl() const;
		void print_fixed_parameters_gmpl() const;
		void print_constraints_gmpl() const { print(); }

	public:
		std::vector<constraint> constraints;

		void set_fixed_param(std::string name, double value) {
			fixed_parameters[name] = value;
		}

		std::map<std::string, double> get_fixed_parameters() const {
			return fixed_parameters;
		}

		std::vector<std::string> get_free_variables() const {
			return get_free_variables(constraints, fixed_parameters);
		}

		// Consider operator overloading later, perhaps...
		void add(const constraint & in) {
			constraints.push_back(in);
		}

		void add(const std::vector<constraint> & in) {
			std::copy(in.begin(), in.end(), 
				std::back_inserter(constraints));
		}

		void add(const constraint_set & other);

		void print() const {
			for (const constraint x: constraints) {
				x.print();
			}
		}

		constraint_set(const std::vector<constraint> in) { add(in); }
		constraint_set(const constraint in) { add(in); }

		constraint_set() {}

		void print_gmpl_program();
};