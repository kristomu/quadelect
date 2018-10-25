#ifndef __TEST_UNIFORM
#define __TEST_UNIFORM

#include <stdlib.h>
#include <string>

#include "tests.h"
#include "../../tools/tools.h"
#include "../../random/random.h"

class Uniform : public Test {
	private:
		double p;
		rng randomizer;

	public:
		Uniform(double p_in, rseed_t seed) : randomizer(seed) {
			assert (p_in >= 0 && p_in <= 1);
			p = p_in;
		}

		double perform_test() { return(randomizer.drand(0, p)); }
		std::string name() const { return ("Uniform(" + dtos(p)+")"); }

		double get_minimum() const { return(0); }
		double get_maximum() const { return(p); }
};

#endif