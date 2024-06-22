#pragma once

#include <cstdlib>
#include <memory>
#include <string>

#include "../stats/coordinate_gen.h"

// A simulator evaluates a voting method and returns a quality score.

class simulator {
	private:
		size_t simulation_count;
		double accumulated_score;

	protected:

		// To actually do any Monte-Carlo fun, we need some
		// source of randomness (pseudo- or quasi-).
		std::shared_ptr<coordinate_gen> entropy_source;

		// Returns linearized.
		virtual double do_simulation() = 0;

		// Turns a linearized value (exact instance or mean)
		// into an exact one by using a nonlinear transformation
		// not depending on any property of the method itself.
		// The default is that the simulator result already is linear,
		// so just pass it unmodified.
		virtual double get_exact_value(double linearized) const {
			return linearized;
		}

	public:
		// If true, bandit searches etc should try to maximize the
		// score. If false, they should try to minimize it (and the
		// "score" is actually a penalty).
		virtual bool higher_is_better() const = 0;

		virtual double simulate(bool get_linearized);

		// For ye bandits
		double get_linearized_mean_score() const;

		// All of these are exact.
		double get_mean_score() const;
		double get_total_score() const;

		size_t get_simulation_count() const {
			return simulation_count;
		}

		void set_entropy_source(std::shared_ptr<coordinate_gen>
			entropy_src_in) {

			entropy_source = entropy_src_in;
		}

		// Provides an upper bound on the sub-Gaussian squared scale
		// parameter (variance proxy) of the simulation result. The lil'UCB
		// bandit algorithm expects the distribution of rewards to be a
		// shifted sub-Gaussian with some unknown expectation. Since the
		// expectation is unknown, the function must give a bound that holds
		// regardless of the expectation. The estimate may exceed the true scale,
		// but if it underestimates it, the multi-armed bandit may fail to find
		// the optimal arm.
		// Note that this is not necessarily the same thing as the variance.
		// See e.g. https://math.stackexchange.com/a/4414653
		virtual double variance_proxy() const = 0;

		virtual void reset() {
			simulation_count = 0;
			accumulated_score = 0;
		}

		simulator(std::shared_ptr<coordinate_gen> entropy_src_in) {
			set_entropy_source(entropy_src_in);
			reset();
		}

		virtual std::string name() const = 0;
};