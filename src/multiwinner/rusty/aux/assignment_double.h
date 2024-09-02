
#include <iostream>
#include <vector>
#include <glpk.h>
#include <assert.h>

#include "tools/tools.h"

using namespace std;

// Solve the forced clustering problem for a single instance.

// BLUESKY: Points system MIP solver.
// Also should check overdetermined case, where clusters > voters

// TODO: Add constraint < previous record (to quickly determine if it's
// infeasible)

class assignment {

	private:
		glp_smcp params;
		glp_prob * lp;

		double num_voters;
		int num_ballots, num_clusters;

		bool use_record;
		int record_constraint;

		void initialize(double n, int u, int k, const vector<double> &
			voters);

	public:
		assignment(double n, int u, int k, const vector<double> &
			voters, bool using_record);

		assignment(double n, int u, int k, const vector<double> &
			voters) : assignment(n, u, k, voters, false) {}

		~assignment();

		void set_constraint(int ballot_num, int cluster_num,
			double value) const;

		double calc_minimum() const;
		int get_status() const;
		bool success() const;

		double get_unknown(int index) const;
};

// n: number of voters
// u: number of unique ballots
// k: number of clusters
void assignment::initialize(double n, int u, int k,
	const vector<double> & voters) {
	assert(n > 0);
	assert(u <= n && u > 0);
	assert(k > 0);

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
	glp_set_obj_dir(lp, GLP_MIN); // Minimum distance

	// Rows = alias variables (p = x1 + x3.. 0 < p < 100)
	// Columns = terms (x1...xn)
	if (use_record) {
		glp_add_rows(lp, u + k + 1);
		record_constraint = u + k + 1;
	} else	{
		glp_add_rows(lp, u + k);
	}

	int counter = 0;

	// Set power constraints: sum of allocated ballot weights
	// must equal number of voters who voted this way.
	// Might be more resistant to vote management if we do
	// 0 <= a <= U_0 (?)
	string name;
	vector<double>::const_iterator pos = voters.begin();
	for (counter = 0; counter < u; ++counter) {
		assert(pos != voters.end());
		name = "pow_" + itos(counter);
		glp_set_row_name(lp, counter + 1, name.c_str());
		glp_set_row_bnds(lp, counter + 1, GLP_FX, *pos, *pos);
		++pos;
	}

	// Set proportionality constraints: each cluster must have
	// just as many allocated to it, so that no cluster is favored.
	double fair_share = n/(double)k;

	for (counter = 0; counter < k; ++counter) {
		name = "prop_" + itos(counter);
		glp_set_row_name(lp, counter + u + 1, name.c_str());
		glp_set_row_bnds(lp, counter + u + 1, GLP_FX, fair_share,
			fair_share);
	}

	// If desired, prepare the record constraint, used to bail out
	// early if we know we can't exceed a past record (alpha-beta-esque?)
	// Leave it free for now.
	if (use_record) {
		glp_set_row_name(lp, record_constraint, "record");
		glp_set_row_bnds(lp, record_constraint, GLP_FR, 0, 0);
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

	// If so desired, construct a record constraint to let the search
	// abort early when we do have a record already.

	glp_load_matrix(lp, lincount-1, ia, ja, ar);

	// And we're all set to receive the distances.
	return;
}

assignment::assignment(double n, int u, int k,
	const vector<double> & voters,
	bool using_record) {

	num_voters = n;
	num_clusters = k;
	num_ballots = u;
	lp = NULL;

	use_record = using_record;

	initialize(n, u, k, voters);
}

assignment::~assignment() {
	if (lp != NULL) {
		glp_delete_prob(lp);
	}
}


void assignment::set_constraint(int ballot_num, int cluster_num,
	double value) const {
	assert(ballot_num < num_ballots);
	assert(cluster_num < num_clusters);

	glp_set_obj_coef(lp, (ballot_num * num_clusters) + cluster_num + 1,
		value);

	if (use_record) {
		// oops. Needs a redesign!
	}
}

double assignment::calc_minimum() const {

	glp_simplex(lp, &params);
	return (glp_get_obj_val(lp));
}

int assignment::get_status() const {
	return (glp_get_status(lp));
}

bool assignment::success() const {
	return (get_status() == GLP_OPT);
}

double assignment::get_unknown(int index) const {

	return (glp_get_col_prim(lp, index+1));
}
