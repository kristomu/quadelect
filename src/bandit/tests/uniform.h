#ifndef __TEST_UNIFORM
#define __TEST_UNIFORM

#include <stdlib.h>
#include <string>

#include "tests.h"
#include "../../tools.h"

class Uniform : public Test {
	private:
		double p;

	public:
		Uniform(double p_in) {
			assert (p_in >= 0 && p_in <= 1);
			p = p_in;
		}

		double perform_test() { return(drand48()*p); }
		std::string name() const { return ("Uniform(" + dtos(p)+")"); }
};

#endif