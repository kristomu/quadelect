#pragma once
#include <eigen3/Eigen/Dense>
#include <vector>

#include "polytope.h"

// A simple convex polytope defined by Ax <= b, with reading and writing.

class simple_polytope : public polytope {
	private:
		Eigen::MatrixXd A;
		Eigen::VectorXd b;

	public:
		const Eigen::MatrixXd & get_A() const {
			return A;
		}
		const Eigen::VectorXd & get_b() const {
			return b;
		}

		void set_A(const Eigen::MatrixXd A_in) {
			A = A_in;
		}
		void set_b(const Eigen::VectorXd b_in) {
			b = b_in;
		}

		simple_polytope(const Eigen::MatrixXd A_in,
			const Eigen::VectorXd b_in) {
			set_A(A_in);
			set_b(b_in);
		}

		simple_polytope() {}
};