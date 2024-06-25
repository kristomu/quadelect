#include "bernoulli.h"
#include "../stats/confidence/as241.h"
#include "../tools/tools.h"

#include <stdexcept>
#include <iostream>

// TODO: Bernoulli simulators don't need linearization. Remove it outright.

double bernoulli_simulator::simulate(bool get_linearized) {
	double result = simulator::simulate(false);

	if (result != 0 && result != 1) {
		throw std::domain_error("Bernoulli simulator returned non-binary result "
			+ dtos(result));
	}

	if (!get_linearized) {
		return result;
	}

	double exact_result = get_exact_value(result, false);

	if (exact_result != 0 && exact_result != 1) {
		throw std::domain_error("Bernoulli simulator: exact result not Bernoulli: "
			+ dtos(exact_result));
	}

	return exact_result;
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

	size_t n = get_simulation_count();
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