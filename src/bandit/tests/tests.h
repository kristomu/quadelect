#ifndef _TEST_TEST
#define _TEST_TEST

#include <string>

// Test is a class that performs some kind of test and returns a reward where
// greater is better.
// Should these be bounded on 0..1?
class Test {
	public:
		virtual double perform_test() = 0;
		virtual std::string name() const = 0;
};

#endif