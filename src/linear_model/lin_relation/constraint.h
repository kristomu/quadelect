// A constraint is a linear relation with a description of what it
// constrains or how it relates to a wider program.

#pragma once
#include <string>
#include <iostream>
#include "lin_relation.h"

class constraint {
	public:
		std::string description;
		lin_relation constraint_rel;

		// AMPL/GMPL format
		void print() const {
			std::cout << "s.t. " << description << ":\n\t";
			constraint_rel.print();
			std::cout << ";\n";
		}
};