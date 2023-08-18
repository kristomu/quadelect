#include "r_sequence.h"

#include <stdexcept>
#include <cmath>

void r_sequence::set_alpha_roots(size_t dimension) {
	double g = 2;

	// We probably need much fewer than 30 iterations to
	// reach convergence, but hey, we can afford the CPU
	// cycles.
	for (size_t iter = 0; iter < 30; ++iter) {
		g = pow(1+g, 1.0/(dimension+1));
	}

	for (size_t dim = 1; dim <= dimension; ++dim) {
		alpha_roots.push_back(1.0/pow(g, dim));
	}
}

std::vector<double> r_sequence::next() {
	for (size_t i = 0; i < current_point.size(); ++i) {
		current_state[i] = fmod(current_state[i]+alpha_roots[i], 1);

		// Add 0.5 as recommended in the article.
		current_point[i] = fmod(current_state[i] + 0.5, 1);
	}

	return current_point;
}