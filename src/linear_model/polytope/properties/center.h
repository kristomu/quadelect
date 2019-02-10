#pragma once

// For determining the Chebyshev center of a polytope (which is a good
// initial point for billiard sampling because one can go almost any
// direction from it).

#include <eigen3/Eigen/Dense>

#include "../polytope.h"
#include "../simple_polytope.h"

class polytope_center {
	private:
		Eigen::MatrixXd chebyshev_augment(const Eigen::MatrixXd & A) const;

	public:
		simple_polytope chebyshev_augment(const polytope & poly_in) const;
		std::pair<double, Eigen::VectorXd> get_chebyshev_center_w_radius(
			const polytope & poly_in) const;

		Eigen::VectorXd get_center(const polytope & poly_in) const {
			return get_chebyshev_center_w_radius(poly_in).second;
		}
};