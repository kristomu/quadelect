#include "simulator.h"
#include <stdexcept>

double simulator::simulate() {

	if (scale_factor_needs_setting) {
		scale_factor = find_scale_factor();
		scale_factor_needs_setting = false;
	}

	double current_score = do_simulation();

	simulation_count++;
	accumulated_score += current_score;

	return scale_factor * current_score;
}

double simulator::get_mean_score() const {
	if (simulation_count == 0) {
		throw std::domain_error("Simulator: division by zero trying to "
			"get mean score!");
	}
	if (scale_factor_needs_setting) {
		throw std::runtime_error("Simulator: Scale factor has not been set!");
	}

	return scale_factor * accumulated_score/simulation_count;
}

double simulator::get_total_score() const {
	if (scale_factor_needs_setting) {
		throw std::runtime_error("Simulator: Scale factor has not been set!");
	}
	return scale_factor * accumulated_score;
}