// Warren D. Smith's Sinkhorn method.

#include "../../pairwise/matrix.h"
#include "method.h"
#include "../method.h"

#include <assert.h>
#include <values.h>
#include <iostream>

#include "sinkhorn.h"


// Like Keener, tolerance specifies the error before convergence is assumed.
// While Sinkhorn converges to 1, to keep compatible with Keener, the input
// parameter has 0 as perfect convergence, just like Keener.

// Add_one adds 1 to all the pairwise matrix entries so that the method
// converges quickly. WDS also considers that more likely to give a good result.

sinkhorn_factor sinkhorn::get_sinkhorn_factor(int max_iterations, const
	std::vector<std::vector<double> > & input_matrix, bool debug) const {

	// This function returns the Sinkhorn factors we use for determining
	// the actual scores. Ordinarily, one can only perform the Sinkhorn
	// normalization operation on certain matrices, but this attempts to
	// fail gracefully when the matrices are not of that form. Therefore,
	// it must make multiple checks for when the sums (our outputs) are
	// about to over- or underflow. Moving the actual Sinkhorn
	// normalization to a separate function makes that much easier.

	int numcands = input_matrix.size();
	std::vector<std::vector<double> > sink_matrix(numcands,
		std::vector<double>(numcands,
			0));
	sinkhorn_factor factor;
	// The row and column vectors must be initialized to 1.
	factor.row = std::vector<double>(numcands, 1);
	factor.col = std::vector<double>(numcands, 1);
	sinkhorn_factor old_factor = factor;

	double maxmin = INFINITY, maxsum, minsum, sum;
	int counter, sec, iter;

	// While we haven't exhausted the number of tries and we're not at the
	// desired tolerance level... continue the iteration.
	for (iter = 0; iter < max_iterations && fabs(maxmin - 1) > tolerance;
		++iter) {
		// Adjust the Sinkhorn matrix.

		for (counter = 0; counter < numcands; ++counter)
			for (sec = 0; sec < numcands; ++sec) {
				double this_val = input_matrix[counter][sec];
				if (add_one) {
					++this_val;
				}

				sink_matrix[counter][sec] = factor.row[counter]
					* factor.col[sec] * this_val;

				// If the col and row values are very large,
				// multiplying them could overflow the result.
				// We check for that here; if it's the case, we
				// abort before the resulting nan destroys the
				// scores.
				if (isfinite(factor.row[counter]) && isfinite(
						factor.col[counter]) &&
					!isfinite(sink_matrix[counter]
						[sec])) {
					return (factor);    // Outta here.
				}
			}

		// Normalize by row.
		maxsum = -INFINITY;
		minsum = INFINITY;

		for (counter = 0; counter < numcands; ++counter) {
			sum = 0;
			for (sec = 0; sec < numcands; ++sec) {
				sum += sink_matrix[counter][sec];
			}

			if (sum != 0 && minsum > sum) {
				minsum = sum;
			}
			if (maxsum < sum) {
				maxsum = sum;
			}

			// If sum is 0, dividing would cause a division by
			// zero, which we don't want.
			if (sum != 0) {
				factor.row[counter] /= sum;

				// The same overflow can happen here, so detect
				// it.
				if (!isfinite(factor.row[counter])) {
					return (old_factor);
				}
			}
		}

		maxmin = maxsum / minsum;

		// ... and by column.
		maxsum = -INFINITY;
		minsum = INFINITY;

		for (counter = 0; counter < numcands; ++counter) {
			sum = 0;
			for (sec = 0; sec < numcands; ++sec)
				if (isfinite(sink_matrix[sec][counter])) {
					sum += sink_matrix[sec][counter];
				}

			if (sum != 0 && minsum > sum) {
				minsum = sum;
			}
			if (maxsum < sum) {
				maxsum = sum;
			}

			if (sum != 0) {
				factor.col[counter] /= sum;
				if (!isfinite(factor.col[counter])) {
					return (old_factor);
				}
			}
		}

		// If this if statement passes, all sums are zero, so we get a
		// tolerance of 0/0 - undefined. Just make it 0 instead by
		// setting minsum to a finite value.
		if (!isfinite(minsum)) {
			minsum = std::min(maxsum, 1.0);
		}

		assert(maxsum >= minsum);

		old_factor = factor;
	}

	return (factor);
}

std::pair<ordering, bool> sinkhorn::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	bool debug = false;

	// Sinkhorn normalization works by alternately normalizing columns
	// and rows of the matrix, storing normalization factors for each.
	// See WDS's paper for details. I've added a few ifs to handle
	// edge cases, to partially resolve things that would otherwise
	// go to infinity.

	// Like Keener, we're going to use the matrix lots of times, so cache
	// it to avoid slowdowns due to vtable lookups in get_magnitude. Only
	// store the hopefuls.

	size_t counter, sec;
	std::vector<int> permitted_candidates;
	permitted_candidates.reserve(input.get_num_candidates());

	for (counter = 0; counter < hopefuls.size(); ++counter)
		if (hopefuls[counter]) {
			permitted_candidates.push_back(counter);
		}

	size_t num_hopefuls = permitted_candidates.size();
	std::vector<std::vector<double> > A(num_hopefuls, std::vector<double>(
			num_hopefuls, 0));

	for (counter = 0; counter < num_hopefuls; ++counter)
		for (sec = 0; sec < num_hopefuls; ++sec)
			if (counter != sec)
				A[counter][sec] = input.get_magnitude(
						permitted_candidates[counter],
						permitted_candidates[sec]);

	// Allocate the row and column factors as well as the Sinkhorn (doubly
	// stochastic) matrix itself.
	/*std::vector<double> sink_col(num_hopefuls, 1), sink_row(num_hopefuls, 1);
	std::vector<std::vector<double> > sink_matrix(num_hopefuls, std::vector<double>(
				num_hopefuls, 0));*/

	int maxiter = 500;
	sinkhorn_factor factor = get_sinkhorn_factor(maxiter, A, debug);

	// Determine the scores. If any go out of bounds because of
	// over/underflow, handle that by normalizing. NOTE! Not compatible
	// with MSVC whose long double is just a double.
	std::vector<double> scores(permitted_candidates.size());
	bool finite_score = true;
	for (counter = 0; counter < permitted_candidates.size() &&
		finite_score; ++counter) {
		scores[counter] = factor.col[counter] / factor.row[counter];
		finite_score = isfinite(scores[counter]);

		if (debug)
			std::cout << "Candidate " << permitted_candidates[counter] <<
				": score is " << scores[counter] << std::endl;
	}

	if (!finite_score) {
		std::vector<long double> ldscores(permitted_candidates.size());
		long double ldmin = INFINITY, ldmax = -INFINITY;

		for (counter = 0; counter < permitted_candidates.size();
			++counter) {
			// TODO: Handle case where someone's row is zero.
			ldscores[counter] = (long double)
				factor.col[counter] / (long double)
				factor.row[counter];
			if (debug)
				std::cout << "Renorm cand scores: " <<
					permitted_candidates[counter] << ": "
					<< factor.col[counter] << " / "
					<< factor.row[counter] << " = "
					<< ldscores[counter] << std::endl;

			ldmin = std::min(ldscores[counter], ldmin);
			ldmax = std::max(ldscores[counter], ldmax);
		}

		for (counter = 0; counter < permitted_candidates.size();
			++counter) {
			scores[counter] = (double)renorm(ldmin, ldmax,
					ldscores[counter], (long double)DBL_MIN,
					(long double)DBL_MAX);
			if (debug)
				std::cout << "Renormed " << permitted_candidates[
						counter] << ": " << scores[counter]
					<< std::endl;
		}
	}

	// All done, return the results.
	return (std::pair<ordering, bool>(ordering_tools().
				indirect_vector_to_ordering(scores,
					permitted_candidates), false));
}

std::string sinkhorn::pw_name() const {

	std::string ret = "Sinkhorn(";
	ret += dtos(tolerance) + ", ";
	if (add_one) {
		ret += "+1)";
	} else {
		ret += "+0)";
	}

	return (ret);
}
