#include "bounding_box.h"
#include "../simplex.h"

#include <iostream>

std::pair<Eigen::VectorXd, Eigen::VectorXd> polytope_bounding_box::
	get_bounding_box(const polytope & poly_in) const {

	int dimension = poly_in.get_dimension();

	// To get the bounding box, we need to solve
	// min x_1, x_2, x_3...
	// to get the left (lower, etc) end of the box; and
	// max x_1, x_2, x_3...
	// to get the left (upper, etc) end of the box.

	// Construct the c vectors for both of these:

	int i;
	std::vector<Eigen::VectorXd> objectives;
	Eigen::VectorXd objective;

	// min
	for (i = 0; i < dimension; ++i) {
		objective = Eigen::VectorXd::Zero(dimension);
		objective(i) = 1;
		objectives.push_back(objective);
	}

	// max
	for (i = 0; i < dimension; ++i) {
		objective = Eigen::VectorXd::Zero(dimension);
		objective(i) = -1;
		objectives.push_back(objective);
	}

	// Get the optimal points and turn them into two vectors.
	std::vector<std::pair<double, Eigen::VectorXd> > outcomes =
		poly_in.linear_program(objectives, false);

	Eigen::VectorXd x_min(dimension), x_max(dimension);

	for (i = 0; i < dimension; ++i) {
		x_min(i) = outcomes[i].second(i);
		x_max(i) = outcomes[i+dimension].second(i);
	}

	return std::pair<Eigen::VectorXd, Eigen::VectorXd>(x_min, x_max);
}

Eigen::VectorXd polytope_bounding_box::get_axis_lengths(
	const polytope & poly_in) const {

	std::pair<Eigen::VectorXd, Eigen::VectorXd> box = get_bounding_box(
		poly_in);

	return box.second - box.first;
}

double polytope_bounding_box::get_max_axis_length(
	const polytope & poly_in) const {

	// Every value will be nonnegative, so the l_inf norm is the maximum
	// element, which gives the length of the bounding box along the
	// widest axis.
	return get_axis_lengths(poly_in).lpNorm<Eigen::Infinity>();
}

#ifdef TEST_BB

int main() {
	simplex u_simplex(2);

	std::pair<Eigen::VectorXd, Eigen::VectorXd> box =
		polytope_bounding_box().get_bounding_box(u_simplex);

	std::pair<Eigen::Vector2d, Eigen::Vector2d> should_be;
	should_be.first << 0, 0;
	should_be.second << 1, 1;

	if (box.first == should_be.first &&
		box.second == should_be.second) {
		std::cout << "Test PASS" << std::endl;
	} else {
		std::cout << "Test FAIL" << std::endl;
	}

	if (polytope_bounding_box().get_max_axis_length(u_simplex) == 1) {
		std::cout << "Test PASS" << std::endl;
	} else {
		std::cout << "Test FAIL" << std::endl;
	}

	return 0;
}

#endif