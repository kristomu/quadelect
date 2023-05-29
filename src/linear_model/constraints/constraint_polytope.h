#pragma once

#include "../polytope/equality_polytope.h"
#include "../lin_relation/constraint_set.h"

// Class for turning a constraint set into a polytope. As a constraint set
// can include both equality and inequality constraints, this polytope is
// based on equality_polytope. It also contains functions to extract
// desired variables from output coordinates (e.g. from billiard sampling
// or a linear program optimal point).

class constraint_polytope : public equality_polytope {
	private:
		// Register new variables into variable_names_idx.
		int add_new_variables(const constraint_set & fixed_param_ref,
			const relation_side & in);

		// Add a particular linear relation to our matrix system, filling
		// in variable_names_idx as necessary.
		void add_relation(const constraint_set & fixed_param_ref,
			lin_relation in);

		// Add all constraints in a constraint set.
		void add_relations(const constraint_set & in);

		std::map<std::string, int> variable_names_idx;

	public:
		int get_num_variables() const;

		constraint_polytope() : equality_polytope() {}

		constraint_polytope(const constraint_set & in) {
			add_relations(in);
			update_reduction();
		}

		int get_variable_index(std::string variable_name) const;

		// For getting indices corresponding to e.g.
		// ABCDbefore ... DCBAbefore or ABCDafter ... DCBAafter.
		std::vector<int> get_variable_indices(
			const std::vector<std::string> & variable_prefixes,
			const std::string suffix) const;

		std::vector<int> get_all_permutations_indices(int numcands,
			const std::string suffix) const;
};