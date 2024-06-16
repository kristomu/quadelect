#pragma once

#include <cstdlib>
#include <memory>
#include <string>

#include "../stats/coordinate_gen.h"

// A simulator evaluates a voting method and returns a quality score.

class simulator {
	protected:
		size_t simulation_count;
		double accumulated_score;
		double scale_factor;
		bool scale_factor_needs_setting;

		// To actually do any Monte-Carlo fun, we need some
		// source of randomness (pseudo- or quasi-).
		std::shared_ptr<coordinate_gen> entropy_source;

		// Determines the scale factor. The default is that
		// no scale factor is required.
		virtual double find_scale_factor() {
			return 1;
		}

		virtual double do_simulation() = 0;

	public:
		// If true, bandit searches etc should try to maximize the
		// score. If false, they should try to minimize it (and the
		// "score" is actually a penalty).
		virtual bool higher_is_better() const = 0;

		virtual double simulate();

		double get_mean_score() const;
		double get_total_score() const;
		size_t get_simulation_count() const {
			return simulation_count;
		}

		void set_entropy_source(std::shared_ptr<coordinate_gen>
			entropy_src_in) {

			entropy_source = entropy_src_in;
		}

		// TODO? Handle unbounded scores? But then the regret
		// could be unbounded too, so bandit search wouldn't really work...

		// NOTE: Even if lower is better, minimum and maximum should
		// give the actual minimum and maximum of the output range.
		virtual double get_minimum() const = 0;
		virtual double get_maximum() const = 0;

		virtual void reset() {
			simulation_count = 0;
			accumulated_score = 0;
			scale_factor = 1;
			scale_factor_needs_setting = true;
		}

		simulator(std::shared_ptr<coordinate_gen> entropy_src_in) {
			set_entropy_source(entropy_src_in);
			reset();
		}

		virtual std::string name() const = 0;
};