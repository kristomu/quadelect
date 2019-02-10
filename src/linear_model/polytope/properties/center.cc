#include "center.h"
#include "../simplex.h"

#include <iostream>

// Calculating the Chebyshev center:

// The Chebyshev center x_c is the center of the largest ball that fits
// inside the polytope. Call its radius r.

// For a half-plane given by A_i, the distance from x_c to A_i is at least
// r if

// sup             <A_i, x_c> + <A_i, u>   <= b_i
// ||u||_2 <= r

// Since the worst case is that <A_i, u> = ||u||_2 * ||A_i||_2, that's equal
// to
//
//		<A_i, x_c> + r ||A_i||_2 <= b_i

// Thus we can find the maximum r by solving the following linear program:

// max r
// subject to
//        <A_i, x_c> + r ||A_i||_2 <= b_i         for all i,

// i.e. in block matrix form,

// max r
// subject to
//        [ A | ||A_i||_2 ] * [x, r] <= b

// and x will give the coordinates of the Chebyshev center.

/////////////////////////////////////////////////////////////////////////

// Augment the A matrix of a polytope, producing A_*, so that the linear
// program max x s.th. A_*x <= b provides the radius of the maximum-radius
// ball that can be contained inside the polytope Ax <= b, and the
// optimal point consists of the coordinates of the center of that ball
// (as well as its radius r).

Eigen::MatrixXd polytope_center::chebyshev_augment(
	const Eigen::MatrixXd & A) const {

	// Get each polytope row's l2 norm into a vector.

	Eigen::VectorXd radius_weight = A.rowwise().norm();

	Eigen::MatrixXd augmented_A(A.rows(), A.cols()+1);
	// Add A
	augmented_A.block(0, 0, A.rows(), A.cols()) = A;
	// Set the additional (rightmost) column to the r_weights vector.
	augmented_A.block(0, augmented_A.cols()-1, augmented_A.rows(), 1) =\
		radius_weight;

	return augmented_A;
}

simple_polytope polytope_center::chebyshev_augment(
	const polytope & poly_in) const {

	return simple_polytope(chebyshev_augment(poly_in.get_A()), poly_in.get_b());
}

std::pair<double, Eigen::VectorXd>
	polytope_center::get_chebyshev_center_w_radius(
	const polytope & poly_in) const {

	// maximize r, i.e. minimize 0 0 0 0 ... -1, since r is the last
	// variable.

	Eigen::VectorXd c = Eigen::VectorXd::Zero(poly_in.get_A().
		cols()+1);
	c[poly_in.get_A().cols()] = -1;

	std::pair<double, Eigen::VectorXd> out =
		chebyshev_augment(poly_in).linear_program(c, false);

	// Strip r away from the output vector because we only want it to
	// contain the coordinates themselves.

	out.second.conservativeResize(out.second.size()-1);

	if (!poly_in.is_inside(out.second)) {
		throw std::out_of_range(
			"polytope_center: Error finding center, possible numerical " 
			"precision\nproblem. Is your polytope matrix full rank?");
	}

	return out;
}

#ifdef TEST

int main() {
	int dimension = 2;
	simplex u_simplex(dimension);
	polytope_center to_test;

	Eigen::VectorXd initial_point = to_test.get_center(u_simplex);
	Eigen::Vector2d should_be;
	should_be << 1 / (2.0 + sqrt(2)), 1 / (2.0 + sqrt(2));

	if (should_be == initial_point) {
		std::cout << "Test PASS" << std::endl;
	} else {
		std::cout << "Test FAIL" << initial_point << std::endl;
	}

	return 0;
}

#endif