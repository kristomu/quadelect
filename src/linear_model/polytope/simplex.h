#pragma once

#include "polytope.h"

#include <iostream>

// The unit n-dimensional simplex.

class simplex : public polytope {
	private:
		Eigen::MatrixXd A;
		Eigen::VectorXd b;

	public:
		const Eigen::MatrixXd & get_A() const { return A; }
		const Eigen::VectorXd & get_b() const { return b; }

		void set_simplex(int dimension) {
			A = Eigen::MatrixXd::Zero(dimension+1, dimension);
			b = Eigen::VectorXd(dimension+1);
			int i;

			for (i = 0; i < dimension; ++i) { b[i] = 0; }
			b[dimension] = 1;

			for (i = 0; i < dimension; ++i) {
				A(i, i) = -1;
				A(dimension, i) = 1;
			}
		}

		simplex(int dimension) { set_simplex(dimension); }
};