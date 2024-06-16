#pragma once

// A specialization of simulators that returns a binary value, and thus
// has a Bernoulli distribution with some unknown mean.

// This property can be used to calculate confidence intervals, which
// can provide useful feedback.

// TODO: Move the confidence interval stuff into stats/confidence instead,
// and replace the incredibly messy Jeffreys prior there with the A-C
// interval.

#include "simulator.h"

class bernoulli_simulator : public simulator {

	private:
		std::pair<double, double> confidence_interval(
			double significance, size_t num_methods) const;

	public:
		// Return the upper and lower endpoints of the
		// confidence interval around the sample mean.
		// p is the p-value of the confidence interval,
		// and the function treats p < 0.5 as 1-p, i.e. a
		// 95% interval can be expressed as 0.95 or 0.05.

		// num_methods denotes the number of total tests being performed,
		// for multiple comparisons. The functions apply a Bonferroni
		// correction to make multiple comparisons possible.
		double get_lower_confidence(double p, size_t num_methods) const;
		double get_upper_confidence(double p, size_t num_methods) const;

		double simulate();

		double get_minimum() const {
			return 0;
		}

		double get_maximum() const {
			return 1;
		}

		bernoulli_simulator(std::shared_ptr<coordinate_gen> entropy_src_in) :
			simulator(entropy_src_in) {}
};