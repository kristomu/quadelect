
#include "errors.h"

#include <vector>
#include <math.h>
#include "tools/tools.h"

double rmse(const std::vector<double> & a, const std::vector<double> & b) {
	double squared = 0;
	int sumcount = std::min(a.size(), b.size());

	for (int counter = 0; counter < sumcount; ++counter) {
		squared += (a[counter]-b[counter])*(a[counter]-b[counter]);
	}

	return (sqrt(squared/(double)sumcount));
}


// Loosemore-Hanby, scaled by number of entries
double lhi(const std::vector<double> & a, const std::vector<double> & b) {

	double absdiff = 0;
	int sumcount = std::min(a.size(), b.size());

	for (int counter = 0; counter < sumcount; ++counter) {
		absdiff += fabs(a[counter]-b[counter]);
	}

	return (absdiff/(double)(sumcount));
}

// Sainte-Laguë Index.
double sli(const std::vector<double> & quantized, const
	std::vector<double> & pop_profile) {

	double sum = 0;
	double slack = 1e-9; // to prevent division by zero

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
		sum += square(quantized[counter]-pop_profile[counter])/
			(pop_profile[counter] + slack);
	}

	return (sum/(double)sumcount);
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
		sum += real[counter] * log(quantized[counter]);
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
