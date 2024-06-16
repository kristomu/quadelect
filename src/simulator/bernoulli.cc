#include "bernoulli.h"
#include "../stats/confidence/as241.h"
#include "../tools/tools.h"

#include <stdexcept>
#include <iostream>

double bernoulli_simulator::simulate() {
	if (scale_factor_needs_setting) {
		scale_factor = find_scale_factor();
		scale_factor_needs_setting = false;
	}

	double current_score = do_simulation();

	if (current_score != 0 && current_score != 1) {
		throw std::domain_error("Bernoulli simulator returned non-binary result "
			+ dtos(current_score));
	}

	simulation_count++;
	accumulated_score += current_score;

	return scale_factor * current_score;
}

std::pair<double, double> bernoulli_simulator::confidence_interval(
	double significance, size_t num_methods) const {

	if (significance > 0.5) {
		significance = 1-significance;
	}

	num_methods = 1;

	// Apply Bonferroni correction.
	significance /= num_methods;

	// Calculate the Z-score for the given significance.
	double z = ppnd7(1-significance/2);

	// Calculate an approximate binomial proportion confidence interval
	// using the Agresti–Coull interval.

	size_t n = simulation_count;
	size_t successes = get_mean_score() * n;

	double n_mark = n + z*z;
	double p_mark = 1/n_mark * (successes + (z*z)/2.0);
	double W = z * sqrt(p_mark/n_mark * (1 - p_mark));

	double lower = std::max(0.0, p_mark - W);
	double upper = std::min(1.0, p_mark + W);

	return std::pair<double, double>(lower, upper);
}

double bernoulli_simulator::get_lower_confidence(
	double significance, size_t num_methods) const {

	return confidence_interval(significance, num_methods).first;
}

double bernoulli_simulator::get_upper_confidence(
	double significance, size_t num_methods) const {

	return confidence_interval(significance, num_methods).second;
}