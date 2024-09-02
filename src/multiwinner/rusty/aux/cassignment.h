// KLUDGE to get both CFC-Range and FC-Kemeny.

#pragma once

#include <iostream>
#include <vector>
#include <glpk.h>
#include <assert.h>

#include "tools/tools.h"
#include "common/ballots.h"

using namespace std;

// Solve the forced clustering problem for a single instance.

// BLUESKY: Points system MIP solver.
// Also should check overdetermined case, where clusters > voters

// Remember: must remove the other cassignment class or there will be a clash.

class cassignment {

	private:
		bool inited;

		glp_smcp params;
		glp_prob * lp;

		double num_voters;
		int num_ballots, num_clusters;

	public:
		void initialize(double n, int u, int k,
			const list<ballot_group> & voters,
			bool maximize);

		cassignment(double n, int u, int k, const list<ballot_group> &
			voters, bool maximize);
		cassignment();

		~cassignment();

		void set_constraint(int ballot_num, int cluster_num,
			double value) const;

		double calc_optimum(bool debug) const;
		double calc_optimum() const;
		int get_status() const;
		bool success() const;

		double get_unknown(int index) const;
};

// n: number of voters
// u: number of unique ballots
// k: number of clusters
void cassignment::initialize(double n, int u, int k,
	const list<ballot_group> & voters, bool maximize) {
	assert(n > 0);
	assert(u <= n && u > 0);
	assert(k > 0);

	num_voters = n;
	num_clusters = k;
	num_ballots = u;

	if (lp != NULL) {
		glp_delete_prob(lp);
		lp = NULL;
	}

	// Set parameters so we don't get a lot of messages
	glp_init_smcp(&params);
	params.msg_lev = GLP_MSG_OFF;

	// Create problem.
	lp = glp_create_prob();
	assert(lp != NULL);
	glp_set_prob_name(lp, "clustering");
	// If we're acting to maximize a score (a beneficial thing), such as
	// with CFC-Range, then we should set this to GLP_MAX. If we're acting
	// to minimize an error measure, such as with CFC-Kemeny, then we should
	// set this to GLP_MIN.
	if (maximize) {
		glp_set_obj_dir(lp, GLP_MAX);    // Maximum score
	} else {
		glp_set_obj_dir(lp, GLP_MIN);    // Minimum distance
	}

	// Rows = alias variables (p = x1 + x3.. 0 < p < 100)
	// Columns = terms (x1...xn)
	glp_add_rows(lp, u + k);

	int counter = 0;

	// Set power constraints: sum of allocated ballot weights
	// must equal number of voters who voted this way.
	// Might be more resistant to vote management if we do
	// 0 <= a <= U_0 (? seems to have no effect)
	string name;
	list<ballot_group>::const_iterator pos = voters.begin();
	for (counter = 0; counter < u; ++counter) {
		assert(pos != voters.end());
		name = "pow_" + itos(counter);
		glp_set_row_name(lp, counter + 1, name.c_str());
		glp_set_row_bnds(lp, counter + 1, GLP_FX, pos->get_weight(),
			pos->get_weight());
		++pos;
	}

	// Set proportionality constraints: each cluster must have
	// just as many allocated to it, so that no cluster is favored.
	double fair_share = (n/(double)k);

	for (counter = 0; counter < k; ++counter) {
		name = "prop_" + itos(counter);
		glp_set_row_name(lp, counter + u + 1, name.c_str());
		glp_set_row_bnds(lp, counter + u + 1, GLP_FX, fair_share,
			fair_share);
	}

	// Add columns (variables we want to find out)
	glp_add_cols(lp, u * k);

	// Set "must be >= 0" constraints
	for (counter = 0; counter < u * k; ++counter) {
		name = "x_" + itos(counter);
		glp_set_col_name(lp, counter + 1, name.c_str());
		glp_set_col_bnds(lp, counter + 1, GLP_LO, 0, 0);
	}

	// Row (constraint #) indices in ia, column indices (var #) in ja,
	// multipliers (always 1) in ar.

	//int ia[u*k], ja[u*k];
	//double ar[u*k];

	int ia[u*k*2], ja[u*k*2];
	double ar[u*k*2];

	// Now actually link the constraints to the right variables.
	// First the ballot power constraints (includes a single row)...
	int sec, lincount = 1, constraint_num = 1;

	for (counter = 0; counter < u * k; counter += k) {
		for (sec = counter; sec < counter + k; ++sec) {
			ia[lincount] = constraint_num;
			ja[lincount] = sec + 1;
			ar[lincount] = 1; // no multiples!
			++lincount;
		}
		++constraint_num;
	}

	// Then the proportionality constraints (includes a single column)...
	for (counter = 0; counter < k; ++counter) {
		for (sec = counter; sec < u * k; sec += k) {
			ia[lincount] = constraint_num;
			ja[lincount] = sec + 1;
			ar[lincount] = 1;
			++lincount;
		}
		++constraint_num;
	}

	glp_load_matrix(lp, lincount-1, ia, ja, ar);

	// And we're all set to receive the distances.
	inited = true;
	return;
}

cassignment::cassignment(double n, int u, int k,
	const list<ballot_group> & voters, bool maximize) {
	lp = NULL;

	initialize(n, u, k, voters, maximize);
	inited = true;
}

cassignment::cassignment() {
	inited = false;
	lp = NULL;
}

cassignment::~cassignment() {
	if (lp != NULL) {
		glp_delete_prob(lp);
	}
}


void cassignment::set_constraint(int ballot_num, int cluster_num,
	double value) const {
	assert(ballot_num < num_ballots);
	assert(cluster_num < num_clusters);
	assert(inited);

	glp_set_obj_coef(lp, (ballot_num * num_clusters) + cluster_num + 1,
		value);
}

double cassignment::calc_optimum(bool debug) const {

	assert(inited);

	glp_simplex(lp, &params);
	if (debug) {
		glp_write_lp(lp, NULL, "debug.lp");
	}
	return (glp_get_obj_val(lp));
}

double cassignment::calc_optimum() const {
	return (calc_optimum(false));
}

int cassignment::get_status() const {
	assert(inited);
	return (glp_get_status(lp));
}

bool cassignment::success() const {
	return (get_status() == GLP_OPT);
}

double cassignment::get_unknown(int index) const {
	assert(inited);

	return (glp_get_col_prim(lp, index+1));
}