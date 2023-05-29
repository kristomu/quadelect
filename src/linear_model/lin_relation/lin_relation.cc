// TODO when I import this into quadelect: replace void print with
// something that uses itos and returns a string.

#include "lin_relation.h"
#include <iostream>
#include <string>
#include <math.h>

void relation_side::print() const {

	bool first = true;
	bool printed_a_variable = false;
	bool negative;

	for (const std::pair<std::string, double> & var: weights) {

		negative = var.second < 0;

		if (negative) {
			std::cout << " - ";
		}

		if (!first && !negative) {
			std::cout << " + ";
		}

		if (fabs(var.second) != 1) {
			std::cout << fabs(var.second) << " * ";
		}
		std::cout << var.first;
		first = false;
		printed_a_variable = true;
	}

	if (fabs(constant) > 1e-12 || !printed_a_variable) {
		if (constant > 0 && printed_a_variable) {
			std::cout << " + ";
		}
		std::cout << constant;
	}
}

void lin_relation::print() const {

	lhs.print();
	switch (type) {
		case LREL_LE: std::cout << " <= "; break;
		case LREL_GE: std::cout << " >= "; break;
		case LREL_EQ: std::cout << " = "; break;
	}
	rhs.print();

}
