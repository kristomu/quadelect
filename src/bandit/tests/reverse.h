#ifndef __TEST_REVERSE
#define __TEST_REVERSE

#include <stdlib.h>
#include <string>

#include "tests.h"
#include "../../tools/tools.h"

// For finding the worst test instead of the best.

class ReverseTest : public Test {
	private:
		Test * original;

	public:
		ReverseTest(Test * original_test) {
			original = original_test;
		}

		double perform_test() {
			return (
					original->get_maximum()-original->perform_test());
		}
		std::string name() const {
			return ("Rev-" + original->name());
		}

		double get_minimum() const {
			return (0);
		}
		double get_maximum() const {
			return (
					original->get_maximum()-original->get_minimum());
		}
};

#endif