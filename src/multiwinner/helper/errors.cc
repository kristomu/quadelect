#include "errors.h"

#include <vector>
#include <numeric>
#include <iterator>
#include <math.h>
#include "tools/tools.h"

#include <iostream>

// Hmm, these might not be valid after all if they all assume mutual
// exclusive categories (due to the nature of party list). I should
// investigate if that's the case.

double gallagher(const std::vector<double> & a,
	const std::vector<double> & b) {
	double squared = 0;
	int sumcount = std::min(a.size(), b.size());

	for (int counter = 0; counter < sumcount; ++counter) {
		squared += square(a[counter] - b[counter]);
	}

	return (sqrt(0.5 * squared));
}

// G-test. I'm still not sure that this is the correct order.
// Needs testing.
double g_test(const std::vector<double> & quantized,
	const std::vector<double> & pop_profile) {

	double sum = 0;
	size_t elements = std::min(quantized.size(), pop_profile.size());

	for (size_t i = 0; i < elements; ++i) {
		double observed = quantized[i],
			   expected = pop_profile[i];

		if (observed == 0) {
			continue;
		}
		sum += observed * log(observed/expected);
	}

	return 2 * sum;
}

// Loosemore-Hanby
double lhi(const std::vector<double> & a, const std::vector<double> & b) {

	double absdiff = 0;
	int sumcount = std::min(a.size(), b.size());

	for (int counter = 0; counter < sumcount; ++counter) {
		absdiff += fabs(a[counter]-b[counter]);
	}

	return (absdiff/2.0);
}

// Sainte-Laguë Index.
double sli(const std::vector<double> & quantized, const
	std::vector<double> & pop_profile) {

	double sum = 0;
	double slack = 1e-10; // to prevent division by zero

#ifndef NDEBUG

	double a_sum = std::accumulate(quantized.begin(), quantized.end(), 0.0),
		   b_sum = std::accumulate(pop_profile.begin(), pop_profile.end(), 0.0);

	if (fabs(a_sum - b_sum) > 1e-9) {
		std::cout << "Expected " << a_sum << " and " << b_sum << " to be equal.\n";
		throw std::runtime_error("Both observed and expected must use "
			"the same scale!");
	}

#endif

	// TODO: Somehow deal with the "zero vote problem", where
	// some people have an opinion along a dimension, but no
	// candidate does.

	// Currently the slack deals with it by bounding how bad the SLI
	// can get, but it's somewhat of an unsatisfactory solution.

	// OTOH, every method does equally badly, so best, worst, and
	// random disproportionality are all the same. Thus there should
	// be no effect on the VSE because adding a constant cancels out,
	// and the change in the denominator (number of rounds) in the
	// average calculations to approximate expectation also cancels
	// out.

	size_t sumcount = std::min(quantized.size(), pop_profile.size());
	for (size_t counter = 0; counter < sumcount; ++counter) {
		if (quantized[counter] == 0 && pop_profile[counter] == 0) {
			continue;
		}
		sum += square(quantized[counter]-pop_profile[counter])/
			(pop_profile[counter] + slack);
	}

	// Not mean, just raw
	// not that it matters :-)
	return sum;
}

double webster_idx(const std::vector<double> & quantized,
	const std::vector<double> & pop_profile) {

	double fudge = 1e-9; // To avoid a division by zero

	double sum = 0;
	size_t sumcount = std::min(quantized.size(), pop_profile.size());
	for (size_t counter = 0; counter < sumcount; ++counter)
		sum += fabs(1 - quantized[counter]/std::max(
					pop_profile[counter], fudge));

	return (sum/(double)sumcount);
}

double gini(const std::vector<double> & a, const std::vector<double> & b) {

	// n^2. Can be turned linear with a sort in front, but that's not worth
	// it here.

	double sum = 0;
	size_t sumcount = std::min(a.size(), b.size());
	size_t linear = 0;

	for (size_t counter = 0; counter < sumcount; ++counter)
		for (size_t sec = 0; sec < sumcount; ++sec) {
			//if (counter == sec) continue;
			++linear;
			sum += fabs(a[counter]*b[sec] - a[sec]*b[counter]);
		}

	sum /= 200.0;

	return (sum/(double)linear);
}

double entropy(const std::vector<double> & quantized,
	const std::vector<double> & real) {

	double sum = 0;

	for (size_t counter = 0; counter < std::min(
			quantized.size(), real.size()); ++counter) {
		if (real[counter] != 0) {
			sum += real[counter] * log(quantized[counter]);
		}
	}

	return (-sum);
}

double renyi_ent(const std::vector<double> & quantized,
	const std::vector<double> & real,
	double alpha) {

	/*              1            /  p_k^a   \
	   I_a(p, q) = --- log_2 SUM | --------  |
	               1-a        k  \ q_k^(a-1)/    */

	// We use nats instead of bits, per convention of entropy(..).

	double sum = 0;

	for (size_t counter = 0; counter < std::min(
			quantized.size(), real.size()); ++counter) {
		sum += pow(quantized[counter], alpha) / pow(real[counter],
				alpha - 1);
	}

	return (log(sum) / (1.0-alpha));
}
