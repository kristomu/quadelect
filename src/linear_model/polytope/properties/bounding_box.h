#pragma once
#include "../polytope.h"
#include <eigen3/Eigen/Dense>

// For determining the bounding box of a convex polytope.
// This is given as two vectors x_min and x_max so that the linear program
// min x_i s.t. Ax <= b
// equals x_min_i, and similarly for max and x_max_i.
// The bounding box is thus aligned with the axes. (IIRC, calculating a
// bounding box where rotation is free is hard, and in any event isn't
// what we need.)

// The bounding box is used to find the constant M in big-M mixed-
// integer programs such as the one for calculating the l-1 furthest
// distance parameter.

// Lots of stuff could be optimized. Will do that if required.

class polytope_bounding_box {

	public:
		std::pair<Eigen::VectorXd, Eigen::VectorXd> get_bounding_box(
			const polytope & poly_in) const;
		Eigen::VectorXd get_axis_lengths(const polytope & poly_in) const;
		double get_max_axis_length(const polytope & poly_in) const;
};