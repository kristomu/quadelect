#include "tests/tests.h"
#include "bandit.h"

#include <string>

double Bandit::pull() {
	double reward = arm->perform_test();
	accumulated_reward += reward;
	num_pulls += 1;
	return (reward);
}

double Bandit::get_mean() const {
	return (accumulated_reward/get_num_pulls());
}