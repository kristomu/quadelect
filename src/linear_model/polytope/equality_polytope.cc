#include <iostream>
#include <eigen3/Eigen/Dense>
#include <glpk.h>

#include "equality_polytope.h"
#include "simple_polytope.h"

// Input: The convex polytope Cz <= d, Fz = g.
// Output: A polytope of smaller dimension, defined by inequalities alone,
//			where any point inside the polytope can be turned into
//			a point in the input polytope by affine transformation;
//			and the matrix H and vector z_0 that defines that
//			transformation.

equality_reduction equality_polytope::get_equality_reduction() const {

	// To perform the reduction from inequality-and-equality polytope
	// to inequality-only polytope, we construct H and z_0 so that for
	// any vector x, z = Hx + z_0 satisfies Fz = g. Then we simply insert
	// this definition into the output polytope to get

	// Cz <= d, Fz = g		--> 	C(Hx + z_0) <= d
	//						-->		CHx + Cz_0 <= d
	//						-->		CHx <= d - Cz_0

	// thus letting CH = C*H, b = d - Cz_0 works.

	// To find H and z_0, we observe that if we let z_0 be a solution
	// (any solution) to Fz = g, and if z = Hx is always a solution to
	// Fz = 0, then z = Hx + z_0 will also always be a solution to 
	// Fz = g. Setting H to the kernel of the matrix F thus works
	// nicely.

	Eigen::FullPivLU<Eigen::MatrixXd> lu(F);

	equality_reduction output;

	output.H = lu.kernel();			// The null space of F.
	output.z_0 = lu.solve(g);		// It doesn't matter what solution.
	output.reduced = simple_polytope(C * output.H, d - C * output.z_0);

	return output;
}

const Eigen::MatrixXd & equality_polytope::get_A() const {
	update_reduction();
	return reduction.reduced.get_A();
}

const Eigen::VectorXd & equality_polytope::get_b() const {
	update_reduction();
	return reduction.reduced.get_b();
}

Eigen::VectorXd equality_polytope::get_full_coordinates(
	const Eigen::VectorXd & reduced_coordinates) const {

	// If the reduction is stale, then the reduced coordinates may not
	// be valid any longer, so it's better to throw an exception than
	// cause a silent violation of some invariant elsewhere.
	if (reduction_stale) {
		throw std::logic_error("get_full_coordinates: reduced coordinates \
			potentially not valid!");
	}

	return reduction.H * reduced_coordinates + reduction.z_0;
}

// HACK: Not supported yet because the change of bases interacts badly
// with is_binary (e.g. x_0 = z_0 + z_1 and z_0 is binary but z_1 isn't;
// then what is x_0?) So just throw an exception.
std::pair<double, Eigen::VectorXd> equality_polytope::full_poly_mixed_program(
	const Eigen::VectorXd & c, const std::vector<bool> & is_binary,
	bool verbose) const {

	throw std::runtime_error("equality_polytope: mixed program\
		not supported!");
}

std::pair<double, Eigen::VectorXd> equality_polytope::full_poly_linear_program(
	const Eigen::VectorXd & c, 	bool verbose) const {

	update_reduction();

	// Transform c to match the reduced polytope, and then run the LP on
	// that.

	std::pair<double, Eigen::VectorXd> inner_lp_opt =
		reduction.reduced.linear_program(c.transpose() * reduction.H,
			verbose);

	// Transform the optimal coordinates back.

	std::pair<double, Eigen::VectorXd> outer_opt;
	outer_opt.second = get_full_coordinates(inner_lp_opt.second);
	outer_opt.first = c.transpose() * outer_opt.second;

	return outer_opt;
}

#ifdef TEST

/*
 Example:

 C = |-1  0  0  0 | d = |0|  F = |1 1 1 0| g = | 50|
     | 0 -1  0  0 |     |0|      |2 3 4 0|     |158|
     | 0  0 -1  0 |     |0|      |4 3 2 0|     |142|
     | 0  0  0 -1 |     |0|
     | 0  0  0  1 |     |1|
*/


int main()
{
	typedef Eigen::Matrix<double, 5, 4> Matrix5x4;
	typedef Eigen::Matrix<double, 3, 4> Matrix3x4;
	typedef Eigen::Matrix<double, 3, 3> Matrix3x3;

	Matrix3x4 F;
	Matrix5x4 C;
	Eigen::Vector3d g;
	Eigen::VectorXd d(5);

	C << -1, 0, 0, 0, \
		 0, -1, 0, 0, \
		 0, 0, -1, 0, \
		 0, 0, 0, -1, \
		 0, 0, 0, 1;

	d << 0, 0, 0, 0, 1;

	F << 1, 1, 1, 0, \
		 2, 3, 4, 0, \
		 4, 3, 2, 0;

	g << 50, 158, 142;

	equality_polytope equality(C, d, F, g);

	std::cout << "C =\n" << C << "\n" << std::endl;
	std::cout << "d =\n" << d << "\n" << std::endl;
	std::cout << "F =\n" << F << "\n" << std::endl;
	std::cout << "g =\n" << g << "\n" << std::endl;

	std::cout << "---" << std::endl;

	// LP test, kinda kludgy
	// min cTx s.t. Cz <= d, Fz = g.
	// We know the classical solution is (0, 42, 8, 0), so let's try it
	// with a reduced A (i.e. equality constraints folded in).

	// It's particularly kludgy now because this only tests the whole
	// system if linear_program is coded to use the reduction, instead
	// of constructing a separate LP with equality constraints.

	Eigen::Vector4d c;
	c << 0, -1, 0, 0;

  	std::pair<double, Eigen::VectorXd> opt = 
  		equality.ext_linear_program(c, true);
  	
  	Eigen::VectorXd z_opt = opt.second;
  	Eigen::Vector4d should_be;
  	should_be << 0, 42, 8, 0;

  	std::cout << z_opt << std::endl;

  	if (z_opt == should_be) {
  		std::cout << "Test PASS" << std::endl;
  	} else {
  		std::cout << "Test FAIL" << std::endl;
  	}

  	if (F * z_opt == g) {
  		std::cout << "Test PASS" << std::endl;
  	} else {
  		std::cout << "Test FAIL" << std::endl;
  	}
}

#endif