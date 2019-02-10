#include "polytope.h"

#include <glpk.h>
#include <stdexcept>
#include <vector>

#include "../../tools/tools.h"

std::pair<glp_prob *, bool> polytope::create_mixed_program(
	const std::vector<bool> & is_binary) const {

	bool needs_integer_step = false;

	size_t A_size = get_num_halfplanes() * get_dimension();

	// A in sparse representation
	int row_idx[A_size], col_idx[A_size];
	double value[A_size];

	glp_prob *mip;
	mip = glp_create_prob();
	glp_set_prob_name(mip, "eigen_mip");
	glp_set_obj_dir(mip, GLP_MIN);
	glp_add_rows(mip, get_num_halfplanes());
	size_t i;

	// set b
	std::string rowname;
	for (i = 0; i < get_num_halfplanes(); ++i) {
		rowname = "b_" + itos(i);

		glp_set_row_name(mip, i+1, rowname.c_str());
  		glp_set_row_bnds(mip, i+1, GLP_UP, 0.0, get_b()(i));
	}

	// set x
	std::string colname;
	glp_add_cols(mip, get_dimension());
	for (i = 0; i < get_dimension(); ++i) {
		colname = "x_" + itos(i);
		glp_set_col_name(mip, i+1, colname.c_str());
		glp_set_col_bnds(mip, i+1, GLP_FR, 0.0, 0.0);

		if (is_binary[i]) {
			needs_integer_step = true;
			glp_set_col_kind(mip, i+1, GLP_BV);
		}
	}

	// set A
	size_t row, col;
	i = 0;
	for (row = 0; row < get_num_halfplanes(); ++row) {
		for (col = 0; col < get_dimension(); ++col) {
			if (get_A()(row, col) == 0) {
				continue;
			}
			row_idx[1+i] = row+1;
			col_idx[1+i] = col+1;
			value[1+i] = get_A()(row, col);
			++i;
		}
	}

	// load data into program
	glp_load_matrix(mip, i, row_idx, col_idx, value);

	return std::pair<glp_prob *, bool>(mip, needs_integer_step);
}

std::pair<double, Eigen::VectorXd> polytope::run_mixed_program(
	std::pair<glp_prob *, bool> program, const Eigen::VectorXd & c,
	bool verbose) const {

	glp_prob * mip = program.first;
	bool needs_integer_step = program.second;
	size_t i;

	// Guard against reduced coordinates / full coordinates mixup.
	assert((size_t)c.size() == get_dimension());

	if (verbose) {
		glp_term_out(GLP_ON);			// show output
	} else {
		glp_term_out(GLP_OFF);			// suppress output
	}

	// set c
	for (i = 0; i < get_dimension(); ++i) {
		glp_set_obj_coef(mip, i+1, c(i));
	}

	// solve the LP and produce an error message if it couldn't be done.
	if (glp_simplex(mip, NULL) != 0 ||
		glp_get_status(mip) != GLP_OPT) {
		throw std::runtime_error("mixed_program: could not solve LP!");
	}

	// solve the integer program if needed
	if (needs_integer_step) {
		if (glp_intopt(mip, NULL) != 0 ||
			glp_get_status(mip) != GLP_OPT) {
			throw std::runtime_error(
				"mixed_program: could not refine LP to MIP!");
		}
	}

	// return value at optimal point, and that point.
	Eigen::VectorXd x_opt(get_dimension());
	double z;

	if (needs_integer_step) {
		// Get the values from the MIP
		for (i = 0; i < get_dimension(); ++i) {
			x_opt(i) = glp_mip_col_val(mip, i+1);
		}

		z = glp_mip_obj_val(mip);
	} else {
		// Get the values from the LP since we haven't solved any MIP.
		for (i = 0; i < get_dimension(); ++i) {
			x_opt(i) = glp_get_col_prim(mip, i+1);
		}

		z = glp_get_obj_val(mip);
	}

	return std::pair<double, Eigen::VectorXd>(z, x_opt);
}

std::pair<double, Eigen::VectorXd> polytope::mixed_program(
	const Eigen::VectorXd & c, const std::vector<bool> & is_binary,
	bool verbose) const {

	std::pair<glp_prob *, bool> program = create_mixed_program(is_binary);

	std::pair<double, Eigen::VectorXd> result = run_mixed_program(program,
		c, verbose);

	// Free the memory associated with the problem.
	glp_delete_prob(program.first);
	glp_free_env();

	return result;
}

std::vector<std::pair<double, Eigen::VectorXd> > polytope::mixed_program(
	const std::vector<Eigen::VectorXd> & c_objectives,
	const std::vector<bool> & is_binary, bool verbose) const {

	std::pair<glp_prob *, bool> program = create_mixed_program(is_binary);

	std::vector<std::pair<double, Eigen::VectorXd> > results;

	for (const Eigen::VectorXd & c: c_objectives) {
		results.push_back(run_mixed_program(program, c, verbose));
	}

	glp_delete_prob(program.first);
	glp_free_env();

	return results;
}

std::pair<double, Eigen::VectorXd> polytope::linear_program(
	const Eigen::VectorXd & c, 	bool verbose) const {

	return mixed_program(c, std::vector<bool>(c.rows(), false), verbose);
}

std::vector<std::pair<double, Eigen::VectorXd> > polytope::linear_program(
	const std::vector<Eigen::VectorXd> & c_objectives,
	bool verbose) const {

	return mixed_program(c_objectives, std::vector<bool>(
		c_objectives[0].rows(), false), verbose);
}


bool polytope::is_inside(const Eigen::VectorXd & x) const {
	Eigen::VectorXd signed_distance = get_A() * x - get_b();

	for (int i = 0; i < signed_distance.rows(); ++i) {
		if(signed_distance(i) > 0) {
			return false;
		}
	}
	return true;
}

int polytope::get_rank() const {
	Eigen::FullPivLU<Eigen::MatrixXd> lu(get_A());
	return lu.rank();
}

bool polytope::is_full_rank() const {

	return get_rank() == std::min(get_A().cols(), get_A().rows());
}
