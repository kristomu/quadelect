#include "simulator.h"
#include <stdexcept>

double simulator::simulate(bool get_linearized) {

	double current_score = do_simulation();

	simulation_count++;
	accumulated_score += current_score;

	if (get_linearized) {
		return current_score;
	} else {
		return get_exact_value(current_score);
	}
}

double simulator::get_linearized_mean_score() const {
	if (simulation_count == 0) {
		throw std::domain_error("Simulator: division by zero trying to "
			"get mean score!");
	}

	return accumulated_score/simulation_count;
}


double simulator::get_mean_score() const {
	return get_exact_value(get_linearized_mean_score());
}

double simulator::get_total_score() const {
	return get_mean_score() * simulation_count;;
}