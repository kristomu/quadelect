#pragma once

#include "polytope.h"
#include "simple_polytope.h"

// Represents the polytope CHx <= b, with z = Hx + z_0 satisfying
// Cz <= d, Fz = g.

struct equality_reduction {
	simple_polytope reduced; // (CH, b)
	Eigen::MatrixXd H;
	Eigen::VectorXd z_0;
};

// A polytope represented by the constraints Cz <= d, Fz = g.
// The polytope also exposes a reduced polytope (Ax <= b) so that
// for all x for which Ax <= b, there exists a corresponding z that
// satisfies Cz <= d, Fz = g.

// This is used for sampling from polytopes with equality constraints,
// since billiard walk does badly under such constraints.

// As far as is possible, the user only interacts with the "extended"
// polytope (the one in terms of z), and the class translates on the fly.
// In some cases (e.g. billiard sampling), it might be necessary to access
// the reduced polytope (the one in terms of x) and convert from x to z;
// thus, anything that deals with only the polytope base class can access
// the reduced polytope through get_A and similar.

class equality_polytope : public polytope {
	protected:
		mutable equality_reduction reduction;
		mutable bool reduction_stale;

		Eigen::MatrixXd C, F;
		Eigen::VectorXd d, g;

		equality_reduction get_equality_reduction() const;

	public:
		const Eigen::MatrixXd & get_A() const;
		const Eigen::VectorXd & get_b() const;

		void update_reduction() const {
			if (reduction_stale) {
				reduction = get_equality_reduction();
				reduction_stale = false;
			}
		}

		void set_C(const Eigen::MatrixXd C_in) {
			C = C_in;
			reduction_stale = true;
		}
		void set_d(const Eigen::VectorXd d_in) {
			d = d_in;
			reduction_stale = true;
		}
		void set_F(const Eigen::MatrixXd F_in) {
			F = F_in;
			reduction_stale = true;
		}
		void set_g(const Eigen::VectorXd g_in) {
			g = g_in;
			reduction_stale = true;
		}

		equality_polytope() {
			reduction_stale = true;
		}

		equality_polytope(const Eigen::MatrixXd C_in,
			const Eigen::VectorXd d_in, const Eigen::MatrixXd F_in,
			const Eigen::VectorXd g_in) {

			set_C(C_in);
			set_d(d_in);
			set_F(F_in);
			set_g(g_in);
			update_reduction();
		}

		Eigen::VectorXd get_full_coordinates(const Eigen::VectorXd &
			reduced_coordinates) const;

		// TODO: Determine how we should transparently expose both the
		// "full polytope" (e.g. be able to specify c in terms of all
		// variables) and the "reduced polytope" (with equalities folded into
		// inequalities) at the same time. For now, call everything
		// based around the full one full_poly.
		virtual std::pair<double, Eigen::VectorXd> full_poly_mixed_program(
			const Eigen::VectorXd & c, const std::vector<bool> & is_binary,
			bool verbose) const;

		virtual std::pair<double, Eigen::VectorXd> full_poly_linear_program(
			const Eigen::VectorXd & c, 	bool verbose) const;
};