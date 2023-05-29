#include "tests/tests.h"
#include "binomial.h"

#include <stdexcept>

double BinomialBandit::pull() {
	++num_pulls;

	double reward = arm->perform_test();

	if (reward == 0) {
		return (0);
	}
	if (reward == 1) {
		++num_successes;
		++accumulated_reward;
		return (1);
	}

	// Perhaps better to make a BernoulliTest?
	throw std::range_error("BinomialBandit with test that isn't Bernoulli!");
}