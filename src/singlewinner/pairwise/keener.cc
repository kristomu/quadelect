// Keener Eigenvector method. Let's hope I get it right :-)

#include "keener.h"

#include <assert.h>
#include <iostream>

using namespace std;

// Tolerance specifies the, well, tolerance before it assumes convergence.
// Add_one adds 1 to all the Condorcet matrix entries so that the method always
// converges (and geometrically). Normalize_diagonal sets the diagonal s.th.
// U_aa = num voters - sum (b != a) U_ba for all a, which is required in order
// for the ratings to have a proper Markovian meaning.

// KEENER, James P. The Perron–Frobenius theorem and the ranking of football 
// teams. SIAM review, 1993, 35.1: 80-93.

// Observations from Yee: norm = true leads to some very weird results.

pair<ordering, bool> keener::pair_elect(const abstract_condmat & input, 
		const vector<bool> & hopefuls, cache_map * cache,
		bool winner_only) const {

	// Because the iteration accesses the matrix so many times, it's
	// preferrable to dump it into an actual matrix so we won't have
	// to pay for the indirect calls every time.
	vector<int> permitted_candidates;
	permitted_candidates.reserve(input.get_num_candidates());

	size_t counter;

	bool debug = false;

	for (counter = 0; counter < hopefuls.size(); ++counter)
		if (hopefuls[counter])
			permitted_candidates.push_back(counter);

	vector<vector<double> > A(permitted_candidates.size(),
			vector<double>(permitted_candidates.size(), 0));

	// Copy from the Condorcet matrix to A. We copy in the "opposite"
	// manner to what is usual, so that normalizing diagonals become easy
	// if that parameter is set.
	size_t x, y;
	size_t num_hopefuls = permitted_candidates.size();
	for (x = 0; x < num_hopefuls; ++x) {
		double running_count = 0;
		for (y = 0; y < num_hopefuls; ++y) {
			double curval = 0;
			if (add_one)
				++curval;

			// If x == y, then it's either 0 or 1 depending on
			// add_one. In either case, it doesn't count towards
			// the running count because if we use it, it will
			// overwrite the diagonal anyway.
			if (x != y) {
				curval += input.get_magnitude(
					permitted_candidates[y], 
					permitted_candidates[x]);
				running_count += curval;
			}

			A[y][x] = curval;
		}

		// If we want the Markov approach, this can actually be any
		// constant you want. Maybe having one that's large enough that
		// all these values are positive would be preferable.
		if (normalize_diagonal)
			A[x][x] = input.get_num_voters() - running_count;
	}

	// Okay, we have the matrix. Now make use of the power method to find
	// the Perron eigenvector.

	vector<double> scores(permitted_candidates.size(), 1),
		tmp(permitted_candidates.size());
	double oldnf = INFINITY, norm_factor = 0;

	// To ensure termination, we give up after a certain number of
	// iterations. This will then produce a very strange output, which
	// will cause a bad BR, and thus we know the method isn't suited to
	// that particular type.
	int maxiter = 1000;

	for (int iter = 0; iter < maxiter && 
			fabs(norm_factor - oldnf) > tolerance; ++iter) {
		fill(tmp.begin(), tmp.end(), 0);

		for (y = 0; y < num_hopefuls; ++y)
			for (x = 0; x < num_hopefuls; ++x)
				tmp[y] += scores[x] * A[y][x];

		oldnf = norm_factor;
		norm_factor = 0;

		for (counter = 0; counter < num_hopefuls; ++counter)
			norm_factor += tmp[counter];

		if (norm_factor != 0)
			for (counter = 0; counter < num_hopefuls; ++counter)
				scores[counter] = tmp[counter] / norm_factor;

		if (debug)
			cout << "CONVERGENCE: " << pw_name() << ": " << 
				fabs(norm_factor - oldnf) << endl;
	}

	// Okay, we now have our scores. Spool into an order.
	// TODO: Somehow mark that this actually returns scores, not just
	// rank values.
	
	ordering out;

	for (counter = 0; counter < num_hopefuls; ++counter)
		out.insert(candscore(permitted_candidates[counter], 
					scores[counter]));

	return(pair<ordering, bool>(out, false));
}

string keener::pw_name() const {

	string ret = "Keener(";
	ret += dtos(tolerance) + ", ";
	if (add_one)
		ret += "+1, ";
	else	ret += "+0, ";
	if (normalize_diagonal)
		ret += "norm)";
	else	ret += "raw)";

	return(ret);
}
