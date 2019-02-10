#pragma once
#include <eigen3/Eigen/Dense>
#include <vector>
#include <glpk.h>

// Convex polytope defined by Ax <= b. This is a base class that
// permits reading but not writing; and even reading is pure virtual.

class polytope {
	private:
		std::pair<glp_prob *, bool> create_mixed_program(
			const std::vector<bool> & is_binary) const;
		std::pair<double, Eigen::VectorXd> run_mixed_program(
			std::pair<glp_prob *, bool> program,
			const Eigen::VectorXd & c, bool verbose) const;

	public:
		// Virtual so that complex polytope can update A and b from
		// other matrices if they've gone stale.
		virtual const Eigen::MatrixXd & get_A() const = 0;
		virtual const Eigen::VectorXd & get_b() const = 0;

		virtual size_t get_num_halfplanes() const {
			return get_A().rows(); }
		virtual size_t get_dimension() const { return get_A().cols(); }

		// Solves a mixed integer program
		// min cTx s.t. Ax <= b
		// with x_i as binary for all i for which is_binary[i] is true.

		std::pair<double, Eigen::VectorXd> mixed_program(
			const Eigen::VectorXd & c, const std::vector<bool> & is_binary,
			bool verbose) const;

		// This one goes through multiple objectives much more quickly
		// than repeatedly calling the former would.
		std::vector<std::pair<double, Eigen::VectorXd> > mixed_program(
			const std::vector<Eigen::VectorXd> & c_objectives,
			const std::vector<bool> & is_binary,
			bool verbose) const;

		// Solves the linear program
		// min cTx s.t. Ax <= b

		std::vector<std::pair<double, Eigen::VectorXd> > linear_program(
			const std::vector<Eigen::VectorXd> & c_objectives,
			bool verbose) const;

		std::pair<double, Eigen::VectorXd> linear_program(
			const Eigen::VectorXd & c, 	bool verbose) const;

		// Used by polytopes that both expose A (which use reduced
		// coordinates) and some other matrices (which use full
		// coordinates). A bit of a hack but oh well...
		virtual Eigen::VectorXd get_full_coordinates(
			const Eigen::VectorXd & reduced_coordinates) const {
			return reduced_coordinates; }

		bool is_inside(const Eigen::VectorXd & x) const;

		// Neither the billiard sampler nor the Chebyshev center
		// calculation works unless the matrix has full rank, so
		// add functions to determine that here.

		int get_rank() const;
		bool is_full_rank() const;

};