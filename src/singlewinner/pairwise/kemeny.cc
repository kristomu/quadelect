// Kemeny. This method is, unfortunately, NP-hard. We do as best as we can by
// making use of the GLPK to solve it through integer programming. (We might
// handle the linear programming relaxation too, later: just solve and then
// output the X>Y weights into Ranked Pairs.)

// The exact integer program will be given in comments within the pair_elect
// function itself.

#include "../../pairwise/matrix.h"
#include "method.h"
#include "kemeny.h"

#include <iostream>
#include <vector>
#include <glpk.h>
#include <assert.h>

using namespace std;

vector<vector<bool> > kemeny::solve_kemeny(const abstract_condmat & input,
		const vector<bool> & hopefuls, bool debug) const {

	// The output is an adjacency matrix where [a][b] is true if a beats b
	// in the transitive Kemeny ordering.

	// --- //

	// To find the Kemeny scores of different candidates, we solve the
	// following problem:

	// Define x[i][j] as the parameters to be obtained, c as the number of
	// candidates, and d[i][j] the magnitude of i's win over j (however the
	// input matrix defines it). Then,

	// Maximize
	//	SUM (i = 1...c) SUM (j = 1...c) x[i][j] * d[i][j]
	//
	// Subject to
	//	for any i, j in 1...c, i != j
	//		x[i][j] >= 0, x[i][j] <= 1		(1)
	//		x[i][j] + x[j][i] = 1			(2)
	//						
	//
	//	for any i, j, k in 1...c, i != j != k
	//		x[i][j] + x[j][k] + x[k][i] >= 1	(3)

	// If x[i][j] is 1, then i is ranked higher than j, otherwise not.

	// (1) implies that the parameters are boolean: either i beats j or
	//	i doesn't beat j.
	// (2) means that it can't both be the case that i beats j and j beats
	//	i, or that neither do.
	// (3) enforces the triangle inequality so that cycles are prevented.

	// BLUESKY: Consider how to make equalities work, although that wouldn't
	// be "Kemeny". As it is, glpk breaks ties randomly.

	// TODO: If there are only two candidates, the majority winner wins.
	// We need that special case because there won't be enough space to
	// init the triangle inequality stuff.

	// --- //

	// Initialize the integer program, setting parameters so that we
	// don't get a lot of messages.
	glp_prob * ip;

	// Create the problem.
	ip = glp_create_prob();
	assert (ip != NULL);
	glp_set_prob_name(ip, "Kemeny rank");
	glp_set_obj_dir(ip, GLP_MAX); // Maximum score

	// Rows = alias variables, e.g. p = x[1][2] + x[2][1], then later,
	//		0 <= p <= 1.
	// Columns = terms (x[1][1] ... x[n][n])

	// We first need n^2 alias variables for (2), then n^3 for (3). GLPK
	// handles binary variables by specification, so there's no need to
	// explicitly set that.
	// We could cut the number of alias variables to n^2 - n for (2) and
	// n^3 - n^2 - n for (3) - and even further if not hopeful - but eh.
	// We'll do that later if it's necessary.

	int n = input.get_num_candidates();

	glp_add_rows(ip, n * n * n + n * n);
	glp_add_cols(ip, n * n);

	// We also need ia, ja, and ar parameters for the matrix itself.
	// if ia[x] is 1, ja[x] is 2, and ar[x] is 3, that means that
	// the first (1.) alias parameter contains the second (2.) optimization
	// variable, times 3.

	// For the constraint of (2), we need at most 2 * n * n-1 variables
	// since we aren't including x[a][a] and each row involves two opt.
	// variables (x[a][b] and x[b][a]), both times 1.
	// For the constraint of (3), we need at most 3 * n * n-1 * n-2, since
	// i != j != k.

	int entries = 2 * n * (n-1) + 3 * n * (n-1) * (n-2);
	int * ia = new int[entries + 1];
	int * ja = new int[entries + 1];
	double * ar = new double[entries + 1];
	int running_count = 1;

	int counter, sec, tri;
	string name;

	// Set columns as binary and give them proper names. Excess columns
	// (x[1][1], [2][2] etc) are left alone so they won't slow things too 
	// much.
	for (counter = 0; counter < n; ++counter) {
		if (!hopefuls[counter]) continue;
		for (sec = 0; sec < n; ++sec) {
			if (counter == sec) continue;

			// If we're debugging, it makes sense to label the
			// parameters, so do so. Otherwise, don't, as it takes
			// time.

			if (debug) {
				name = "x[" + itos(counter) + "][" + itos(sec) 
					+ "]";
				glp_set_col_name(ip, counter * n + sec + 1, 
						name.c_str());
			}

			glp_set_col_kind(ip, counter * n + sec + 1, GLP_BV);
		}
	}

	// Set sum constraints: x[i][j] + x[j][i] = 1, so that one of the pair
	// always beats the other, but they don't both.
	for (counter = 0; counter < n; ++counter) {
		if (!hopefuls[counter]) continue;
		for (sec = 0; sec < n; ++sec) {
			if (counter == sec) continue;
			if (!hopefuls[sec]) continue;

			if (debug) {
				name = "direct_" + itos(counter) + "_" + 
					itos(sec);
				glp_set_row_name(ip, counter * n + sec + 1, 
						name.c_str());
			}

			glp_set_row_bnds(ip, counter * n + sec + 1, GLP_FX, 1, 
					1);

			// This row includes x[i][j] times one...
			ia[running_count] = counter * n + sec + 1;
			ja[running_count] = counter * n + sec + 1;
			ar[running_count] = 1;
			++running_count;
			// ... and x[j][i] times one.
			ia[running_count] = counter * n + sec + 1;
			ja[running_count] = sec * n + counter + 1;
			ar[running_count] = 1;
			++running_count;

			}
	}

	assert(running_count < entries + 1);

	// Set triangle inequality constraints: x[i][j] + x[j][k] + x[k][i] >= 1
	// This banishes cycles, verily.

	int offset = n * n; // The first row of these constraints start here.

	for (counter = 0; counter < n; ++counter) {
		for (sec = 0; sec < n; ++sec) {
			if (counter == sec) continue;
			for (tri = 0; tri < n; ++tri) {
				if (tri == sec || tri == counter)
					continue;

				int constraint_no = counter * n * n + sec * n +
					tri;

				if (debug) {
					name = "triangle_" + itos(counter) 
						+ "_" + itos(sec) + "_" 
						+ itos(tri);
					glp_set_row_name(ip, offset + 
							constraint_no + 1,
							name.c_str());
				}

				// Must be >= 1.
				glp_set_row_bnds(ip, offset + constraint_no + 1,
						GLP_LO, 1, 1);

				// This row includes x[i][j] times one...
				ia[running_count] = offset + constraint_no + 1;
				ja[running_count] = counter * n + sec + 1;
				ar[running_count] = 1;
				++running_count;
				// ... x[j][k] times one ...
				ia[running_count] = offset + constraint_no + 1;
				ja[running_count] = sec * n + tri + 1;
				ar[running_count] = 1;
				++running_count;
				// ... and x[k][i] times one.
				ia[running_count] = offset + constraint_no + 1;
				ja[running_count] = tri * n + counter + 1;
				ar[running_count] = 1;
				++running_count;
			}
		}
	}

	// Proceed to load the constraints into the IP solver.
	//cout << "Running count: " << running_count << " of " << 
	//	entries << endl;
	assert (running_count <= entries + 1);
	glp_load_matrix(ip, running_count - 1, ia, ja, ar);

	// Set the objective coefficients.
	for (counter = 0; counter < n; ++counter) {
		if (!hopefuls[counter]) continue;
		for (sec = 0; sec < n; ++sec)
			if (counter != sec && hopefuls[sec])
				glp_set_obj_coef(ip, counter * n + sec + 1,
						input.get_magnitude(counter,
							sec, hopefuls));
	}

	// PHEW!
	// Now all we have to do is set a few parameters and then *SOLVE*. 
	// Since this version of glpk doesn't support presolving of the IP,
	// we have to first solve the linear programming relaxation, then the
	// IP.
	// Note that the LP isn't sufficient for solving a linear relaxed
	// Kemeny rank problem, since the binary constraints are implied, not
	// stated explicitly, and therefore the LP relaxation will ignore them.
	glp_smcp params;
	glp_init_smcp(&params);
	
	if (debug)
		params.msg_lev = GLP_MSG_ON;
	else	params.msg_lev = GLP_MSG_OFF;
	params.presolve = GLP_ON;

	// Solve the relaxation. If it doesn't succeed, return an empty matrix
	// to signal error.
	if (glp_simplex(ip, &params) != 0)
		return(vector<vector<bool> >());

	// Check if the relaxation is already all-integer. If so, there's no
	// need to call the (slow) IP solver. We lose a little by doing this,
	// so if you're running extremely large problems, it might not be worth
	// it, but then you'd probably not notice the slowdown anyway.

	bool already_int = true;
	for (counter = 0; counter < n * n && already_int; ++counter) {
		double retval = glp_get_col_prim(ip, counter+1);
		already_int = (retval == (int)retval);
	}

	glp_iocp io_param;
	glp_init_iocp(&io_param);

	if (debug)
		io_param.msg_lev = GLP_MSG_ON;
	else	io_param.msg_lev = GLP_MSG_OFF;
	
	//io_param.presolve = GLP_ON;
	//io_param.pp_tech = GLP_PP_ALL;

	// Now solve if necessary, and if we get a reasonable answer, 
	// populate the return matrix. If "already_int" is on, we don't have
	// to solve.
	vector<vector<bool> > adjacency;
	if (already_int || (glp_intopt(ip, &io_param) == 0 && 
			glp_mip_status(ip) == GLP_OPT)) {
		adjacency = vector<vector<bool> > (n, vector<bool>(n, false));
		
		for (counter = 0; counter < n; ++counter) {
			if (!hopefuls[counter]) continue;
			for (sec = 0; sec < n; ++sec) {
				if (counter == sec) continue;
				if (!hopefuls[sec]) continue;

				if (already_int)
					adjacency[counter][sec] = (
							glp_get_col_prim(ip,
								counter * n +
								sec + 1) == 1);
				else
					adjacency[counter][sec] = (
							glp_mip_col_val(ip,
								counter * n + 
								sec + 1) == 1);
			}
		}
	}

	// Either the matrix is still empty, in which case there was a failure,
	// or it is now populated, in which case we're done. Thus, deallocate
	// everything and return - the caller will find out.
	
	delete[] ia;
	delete[] ja;
	delete[] ar;
	glp_delete_prob(ip);

	return(adjacency);
}

pair<ordering, bool> kemeny::pair_elect(const abstract_condmat & input,
		const vector<bool> & hopefuls, cache_map & cache, 
		bool winner_only) const {

	// First, get the transitive adjacency matrix for Kemeny.
	vector<vector<bool> > adj = solve_kemeny(input, hopefuls, false);

	// If there's nothing here, abort as that's an error.
	if (adj.empty())
		return(pair<ordering, bool>(ordering(), false));

	// Otherwise, each's score is the number of candidates he ranks above.
	// If I'm going to do a linear relaxation later, I'll have to use a
	// variant of the DFS for Ranked Pairs.

	ordering out;

	for (size_t counter = 0; counter < adj.size(); ++counter) {
		if (!hopefuls[counter]) continue;
		int candcount = 0;
		for (size_t sec = 0; sec < adj.size(); ++sec)
			if (adj[counter][sec])
				++candcount;

		out.insert(candscore(counter, candcount));
	}

	return(pair<ordering, bool>(out, false));
}
