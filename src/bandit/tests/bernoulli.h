#ifndef __TEST_BERNOULLI
#define __TEST_BERNOULLI

#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <string>

#include "tests.h"
#include "../../tools.h"

// TODO?: Use actual OO random number generators so we can control them for
// reproducibility.

class Bernoulli : public Test {
	private:
		double p;

	public:
		Bernoulli(double p_in) {
			assert (p_in >= 0 && p_in <= 1);
			p = p_in;
		}

		double perform_test();
		std::string name() const { return ("Bernoulli(" + dtos(p) + ")");}

		double get_minimum() const { return(0); }
		double get_maximum() const { return(1); }
};

#endif