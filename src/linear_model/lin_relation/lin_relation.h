// Classes for a very flexible representation of a symbolic linear
// relation (inequality or equality). These relations are used to define
// the constraints on variables of interest (e.g. the number of ballots of
// each preference order before and after some candidate is raised, in a
// monotonicity example).

// To be as flexible as possible, the relations directly refer to the names
// of the variables in question, instead of using indices. The relation set
// class then translates a collection of relations to a polytope that can
// be sampled by a billiard walk or optimized with linear programming.

// This class doesn't take into consideration whether a parameter is free or
// fixed; that's done elsewhere.

#pragma once

#include <string>
#include <vector>

// Less than or equal; equal; greater than or equal.
enum relation_type {LREL_LE, LREL_EQ, LREL_GE};

class relation_side {
	public:
		// If a pair is ("ABCD", 3) that means 3 * ABCD is part of that
		// relation side.
		std::vector<std::pair<std::string, double> > weights;
		double constant;
		relation_side() {
			constant = 0;
		}

		void print() const;
};

class lin_relation {
	public:
		relation_side lhs, rhs;
		relation_type type;

		void print() const;
};