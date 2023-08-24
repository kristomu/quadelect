#pragma once

// The abstract base class for everything that generates coordinates in
// n-dimensional space. We use this to unify random and quasirandom generators
// while respecting the latter's requirement that the dimension has to be
// specified in advance.

#include <vector>

class coordinate_gen {
	public:
		// Generates an n-dimensional vector in the hypercube [0..1]^n.
		virtual std::vector<double> get_coordinate(size_t dimension) = 0;
};