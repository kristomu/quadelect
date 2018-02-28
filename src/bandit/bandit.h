#ifndef _BANDIT_BANDIT
#define _BANDIT_BANDIT

#include "tests/tests.h"
#include <string>

class Bandit {
	private:
		double accumulated_reward;
		int num_pulls;
		Test * arm;

	public:
		Bandit(Test * input_arm) : arm(input_arm) {
			accumulated_reward = 0;
			num_pulls = 0;
		}

		double pull();
		int get_num_pulls() const { return(num_pulls); }
		double get_mean() const;

		// Return the name of the arm so we can provide the name of the
		// winner when we've run MAB.
		std::string name() const { return(arm->name());}
};

#endif