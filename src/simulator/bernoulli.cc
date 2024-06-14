#include "bernoulli.h"
#include "../stats/confidence/as241.h"

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

	// Calculate the Z-score for the given significance.
	double z = ppnd7(significance/num_methods);

	// Calculate an approximate binomial proportion confidence interval
	// using the Agresti–Coull interval.

	double p_mean = get_mean_score();
	size_t n = simulation_count;

	double p_mark = (p_mean * n + 0.5 * z * z) / (n + z * z);
	double W = sqrt(p_mark * (1 - p_mark) / (n + z * z));

	return (std::pair<double, double>(p_mark - W, p_mark + W));
}