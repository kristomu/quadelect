// Approximate the maximum-distance l2 diameter of a convex polytope by
// determining the maximum-distance l1 diameter, i.e.

// max ||x-y||_1 subject to Ax <= b, Ay <= b.

// This is a hard problem and thus requires mixed integer programming.
// In the worst case, determining the diameter will take far too long
// a time, but we'll deal with that when it happens.

// The maximum diameter is used to improve mixing times for the billiard
// walk.

// TODO: Fix redundancy (get_simplex and linear_program).

#include <iostream>
#include "distance.h"
#include "../simple_polytope.h"
#include "../simplex.h"
#include "bounding_box.h"

#include <vector>
#include <numeric>
#include <stdexcept>

// To find the l_1 diameter of A, we set up the following problem:

// max sum over i: abs_i

// subject to:
//
//	for i = 1...n
//		abs_i - M_i * (1 - ind_abs.i) <= x_i - y_i	(1)
//		abs_i - M_i * ind_abs.i <= y_i - x_i		(2)
//		abs_i >= 0									(3)
//		ind_abs_i >= 0 binary						(4)
//
//	Ax <= b											(5)
//	Ay <= b											(6)

// or in matrix form:

// x^T   = x_1 .. x_n | y_1 .. y_n | abs_1 .. abs_n | ia_1 ... ia_n ||
//																		b
//																		=
//
//         -----------+------------+----------------+---------------++
// (1) A =  -I        |      I     |        I       |      M * I    ||  M
//         -----------+------------+----------------+---------------++
// (2)       I        |     -I     |        I       |     -M * I    ||  0
//         -----------+------------+----------------+---------------++
// (3)       0        |      0     |       -I       |        0      ||  0
//         -----------+------------+----------------+---------------++
// (4)       0        |      0     |        0       |       -I      ||  0
//         -----------+------------+----------------+---------------++
// (5)       A        |      0     |        0       |        0      ||  b
//         -----------+------------+----------------+---------------++
// (6)       0        |      A     |        0       |        0      ||  b
//         -----------+------------+----------------+---------------++

// M_i must be larger than any possible x_i - y_i; precisely, at least twice
// as large as the maximum x_i - y_i. (See get_l1_diameter for explanation.)

// This can also be used to maximize the l_infinity diameter of A without
// requiring integer programming, by maximizing max i abs.i, but we don't
// do that here since what we'd really need in that case is leximax l_inf
// distance, and doing leximax with integer programming without numerical
// precision issues is hard.

diameter_coords polytope_distance::get_extreme_coords(
	const polytope & poly_in, const Eigen::VectorXd & M_vec,
	bool linear_relaxation) const {

	int n = poly_in.get_A().cols(), m = poly_in.get_A().rows();

	if (M_vec.rows() != n) {
		throw std::invalid_argument("get_extreme_coords: M_vec length"
			" differs from number of columns of A!");
	}

	int prog_rows = 4 * n + 2 * m; // for (1)-(4) and (5) and (6) resp.
	int prog_cols = 4 * n;

	Eigen::MatrixXd diam_prog(prog_rows, prog_cols);
	Eigen::VectorXd diam_b(prog_rows);
	Eigen::VectorXd diam_c(prog_cols);

	// (1)
	diam_prog.block(0, 0, n, n) = -Eigen::MatrixXd::Identity(n, n);
	diam_prog.block(0, n, n, n) = Eigen::MatrixXd::Identity(n, n);
	diam_prog.block(0, 2*n, n, n) = Eigen::MatrixXd::Identity(n, n);
	diam_prog.block(0, 3*n, n, n) = M_vec.asDiagonal();
	// Column vector
	diam_b.block(0, 0, n, 1) = M_vec;
	// (2)
	diam_prog.block(n, 0, n, n) = Eigen::MatrixXd::Identity(n, n);
	diam_prog.block(n, n, n, n) = -Eigen::MatrixXd::Identity(n, n);
	diam_prog.block(n, 2*n, n, n) = Eigen::MatrixXd::Identity(n, n);
	diam_prog.block(n, 3*n, n, n) = -1 * M_vec.asDiagonal();
	diam_b.block(n, 0, n, 1) = Eigen::MatrixXd::Constant(n, 1, 0);
	// (3)
	diam_prog.block(2*n, 0, n, n) = Eigen::MatrixXd::Zero(n, n);
	diam_prog.block(2*n, n, n, n) = Eigen::MatrixXd::Zero(n, n);
	diam_prog.block(2*n, 2*n, n, n) = -Eigen::MatrixXd::Identity(n, n);
	diam_prog.block(2*n, 3*n, n, n) = Eigen::MatrixXd::Zero(n, n);
	diam_b.block(2*n, 0, n, 1) = Eigen::MatrixXd::Constant(n, 1, 0);
	// (4)
	diam_prog.block(3*n, 0, n, 3*n) = Eigen::MatrixXd::Zero(n, 3*n);
	diam_prog.block(3*n, 3*n, n, n) = -Eigen::MatrixXd::Identity(n, n);
	diam_b.block(3*n, 0, n, 1) = Eigen::MatrixXd::Constant(n, 1, 0);
	// (5)
	diam_prog.block(4*n, 0, m, n) = poly_in.get_A();
	diam_prog.block(4*n, n, m, 3*n) = Eigen::MatrixXd::Zero(m, 3*n);
	diam_b.block(4*n, 0, m, 1) = poly_in.get_b();
	// (6)
	diam_prog.block(4*n+m, 0, m, n) = Eigen::MatrixXd::Zero(m, n);
	diam_prog.block(4*n+m, n, m, n) = poly_in.get_A();
	diam_prog.block(4*n+m, 2*n, m, 2*n) = Eigen::MatrixXd::Zero(m, 2*n);
	diam_b.block(4*n+m, 0, m, 1) = poly_in.get_b();

	// minimize -(abs_1 + ... + abs_n)
	diam_c = Eigen::VectorXd::Zero(prog_cols);
	diam_c.block(2*n, 0, n, 1) = Eigen::MatrixXd::Constant(n, 1, -1);

	// ind_abs are all binary
	std::vector<bool> binaries(prog_cols, false);
	for (int i = prog_cols-n; i < prog_cols; ++i) {
		binaries[i] = true;
	}

	simple_polytope diameter_prog_polytope(diam_prog, diam_b);

	// be verbose. (TODO: Change this later, once I've got the IP to be
	// faster, if that's doable).
	std::pair<double, Eigen::VectorXd> output;

	if (linear_relaxation) {
		output = diameter_prog_polytope.linear_program(diam_c, false);
	} else {
		output = diameter_prog_polytope.mixed_program(diam_c, binaries, true);
	}

	// Split up result from the MIP solver into the actual coordinates.
	diameter_coords coords;
	coords.first = output.second.block(0, 0, n, 1);
	coords.second = output.second.block(n, 0, n, 1);

	return coords;
}

diameter_coords polytope_distance::get_extreme_coords(
	const polytope & poly_in, const Eigen::VectorXd & M_vec) const {

	return get_extreme_coords(poly_in, M_vec, do_linear_relaxation);
}

double polytope_distance::get_l1_diameter(const polytope & poly_in,
	const Eigen::VectorXd & M_vec) const {

	diameter_coords coords = get_extreme_coords(poly_in, M_vec);

	return (coords.first - coords.second).lpNorm<1>();
}

// Get a lower bound on the l2 maximum-distance diameter.
double polytope_distance::get_l2_diameter_lb(const polytope & poly_in,
	const Eigen::VectorXd & M_vec) const {

	diameter_coords coords = get_extreme_coords(poly_in, M_vec);

	// Both of these are lower bounds, and we choose the greater of the
	// two.

	// Calculate the l2 distance between the extreme points optimizing
	// l1 distance.
	double one = (coords.first - coords.second).lpNorm<2>();

	// Use the equivalence of lp norms: ||x||_1 <= sqrt(n) * ||x||_2
	double two = (coords.first - coords.second).lpNorm<1>() /
		sqrt(coords.first.cols());

	return std::max(one, two);
}

double polytope_distance::get_l1_diameter(
	const polytope & poly_in) const {

	// M is used in a constraint of the form
	// abs_i - M * (1 - ind_abs.i) <= x_i - y_i
	// we want M to inactivate the abs_i constraint entirely when ind_abs
	// is zero.
	// To do so in the worst case, we must assume that x_i - y_ is
	// -max_axis_length and that abs_i can take a value up to
	// max_axis_length from somewhere else.
	// Thus, to be entirely inactive,  we must have
	// min_axis_length - X <= -min_axis_length,
	// which has the solution X = 2 * min_axis_length. That's why the
	// factor of 2 appears below.

	// We also use the bounding box to construct a lower bound on
	// the maximum diameter. If this lower bound is better than the
	// lower bound we get from the MIP system (in case it's been set to
	// run the LP relaxation only), we use that instead.

	Eigen::VectorXd M_vec = 2 * polytope_bounding_box().get_axis_lengths(
			poly_in);

	double box_estimate = M_vec.maxCoeff();

	return std::max(get_l1_diameter(poly_in, M_vec), box_estimate);
}

double polytope_distance::get_l2_diameter_lb(
	const polytope & poly_in) const {

	Eigen::VectorXd M_vec = 2 * polytope_bounding_box().get_axis_lengths(
			poly_in);

	double box_estimate = M_vec.maxCoeff();

	return std::max(get_l2_diameter_lb(poly_in, M_vec), box_estimate);
}

#ifdef TEST_DIST

int main() {
	int dimension = 2;
	simplex u_simplex(dimension);

	polytope_distance to_test(false);

	double l_1_diam = to_test.get_l1_diameter(u_simplex);
	double should_be_diam = 2;

	if (should_be_diam == l_1_diam) {
		std::cout << "Test PASS" << std::endl;
	} else {
		std::cout << "Test FAIL (" << l_1_diam << ")" << std::endl;
	}

	double l_2_diam = to_test.get_l2_diameter_lb(u_simplex);
	double should_be = sqrt(2);

	if (should_be_diam == l_2_diam) {
		std::cout << "Test PASS" << std::endl;
	} else {
		std::cout << "Test FAIL (" << l_2_diam << ")" << std::endl;
	}



	l_1_diam = to_test.get_l1_diameter(u_simplex);
	should_be_diam = 2;

	if (should_be_diam == l_1_diam) {
		std::cout << "Test PASS" << std::endl;
	} else {
		std::cout << "Test FAIL (" << l_1_diam << ")" << std::endl;
	}

	l_2_diam = to_test.get_l2_diameter_lb(u_simplex);
	should_be = sqrt(2);

	if (should_be_diam == l_2_diam) {
		std::cout << "Test PASS" << std::endl;
	} else {
		std::cout << "Test FAIL (" << l_2_diam << ")" << std::endl;
	}

	return 0;
}

#endif