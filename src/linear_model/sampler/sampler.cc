#ifdef TEST_SAMPLER

#include <iostream>
#include <eigen3/Eigen/Dense>
#include <glpk.h>

#include <random>
#include <numeric>
#include <stdexcept>

#include "sampler.h"
#include "../polytope/polytope.h"
#include "../polytope/simplex.h"

int main() {
	int dimension = 2;

	simplex u_simplex(dimension);
	billiard_sampler<simplex> simplex_walk(u_simplex, false);

	Eigen::VectorXd current_point;

	std::cout << simplex_walk.get_current_point().transpose() << std::endl;

	// Count how many are above the diagonal x=y and how many are below.
	// These should be the same, or close to, if we're sampling uniformly.

	int num_above = 0, num_below = 0;
	//current_ray.orig = initial_point;

	for (int i = 0; i < 10000; ++i) {
		current_point = simplex_walk.billiard_walk();

		if (!u_simplex.is_inside(current_point)) {
			throw std::out_of_range("Billiard sampling test failed!");
		}

		if (current_point(0) < current_point(1)) {
			++num_below;
		} else {
			++ num_above;
		}
	}

	std::cout << "Num below: " << num_below << " and above: " << num_above
		<< std::endl;

	return 0;
}

#endif