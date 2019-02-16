#pragma once
#include "../polytope.h"
#include <eigen3/Eigen/Dense>

// Approximate the maximum-distance l2 diameter of a convex polytope by
// determining the maximum-distance l1 diameter, i.e.

// max ||x-y||_1 subject to Ax <= b, Ay <= b.

// This is a hard problem and thus requires mixed integer programming.
// In the worst case, determining the diameter will take far too long
// a time, but we'll deal with that when it happens.

// The maximum diameter is used to improve mixing times for the billiard
// walk.

typedef std::pair<Eigen::VectorXd, Eigen::VectorXd> diameter_coords;

class polytope_distance {
	private:
		bool do_linear_relaxation;

		diameter_coords get_extreme_coords(const polytope & poly_in,
			const Eigen::VectorXd & M_vec, bool linear_relaxation) const;

	public:
		void set_linear_relaxation(bool use_relaxation) {
			do_linear_relaxation = use_relaxation;
		}

		polytope_distance(bool use_relaxation) {
			set_linear_relaxation(use_relaxation);
		}

		polytope_distance() : polytope_distance(false) {}

		diameter_coords get_extreme_coords(const polytope & poly_in,
			const Eigen::VectorXd & M_vec) const;
		diameter_coords get_extreme_coords(const polytope & poly_in,
			double M) const {
			int n = poly_in.get_A().cols();

			return get_extreme_coords(poly_in,
				Eigen::MatrixXd::Constant(n, 1, M));
		}

		double get_l1_diameter(const polytope & poly_in,
			const Eigen::VectorXd & M_vec) const;

		double get_l2_diameter_lb(const polytope & poly_in,
			const Eigen::VectorXd & M_vec) const;

		// Using bounding boxes to infer M and for lower bounds.

		double get_l1_diameter(const polytope & poly_in) const;
		double get_l2_diameter_lb(const polytope & poly_in) const;
};