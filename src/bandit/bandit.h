#pragma once

#include "tests/tests.h"
#include <string>
#include <memory>

class Bandit {
	protected:
		double accumulated_reward;
		int num_pulls;
		std::shared_ptr<Test> arm;

	public:
		Bandit(std::shared_ptr<Test> input_arm) : arm(input_arm) {
			accumulated_reward = 0;
			num_pulls = 0;
		}

		virtual double pull();
		int get_num_pulls() const {
			return (num_pulls);
		}
		double get_mean() const;

		// Get the range of the random variable. This is required to
		// calculate the lil'USB variance parameter in absence of actual
		// variance information.
		double get_minimum() const {
			return (arm->get_minimum());
		}
		double get_maximum() const {
			return (arm->get_maximum());
		}

		// Return the name of the arm so we can provide the name of the
		// winner when we've run MAB.
		std::string name() const {
			return (arm->name());
		}
};