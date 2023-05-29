#ifndef _BANDIT_BINOMIAL
#define _BANDIT_BINOMIAL

#include "tests/tests.h"
#include "bandit.h"

// A multiarmed bandit connected to a Bernoulli test.
// The point of having this be a separate class is that we can infer
// confidence intervals on the bandit results without having to make
// assumptions or use hacks that handle both binomial and non-binomial
// situations.

class BinomialBandit : public Bandit {
	private:
		int num_successes;

	public:
		BinomialBandit(Test * input_arm) : Bandit(input_arm) {
			num_successes = 0;
		}

		double pull();
		int get_num_successes() const {
			return (num_successes);
		}
};

#endif