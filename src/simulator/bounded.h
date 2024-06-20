#pragma once

#include "simulator.h"

// A specialization of the simulator class for simulators with bounded
// results/scores.

class bounded_simulator : public simulator {

	protected:
		// These should be set to give the actual minimum
		// and maximum bounds of the results, regardless of
		// whether lower or higher is better.
		virtual double get_minimum() const = 0;
		virtual double get_maximum() const = 0;

		virtual double variance_proxy() const {
			// The variance proxy of a bounded distribution is
			// (max-min)^2/4.

			double range = get_maximum()-get_minimum();

			return range*range/4;
		}

	public:
		bounded_simulator(std::shared_ptr<coordinate_gen> entropy_src_in) :
			simulator(entropy_src_in) {}
};