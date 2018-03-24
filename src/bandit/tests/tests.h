#ifndef _TEST_TEST
#define _TEST_TEST

#include <string>

// Test is a class that performs some kind of test and returns a reward where
// greater is better.
// Should these be bounded on 0..1? No longer required as long as they're
// bounded in general.
class Test {
	public:
		virtual double perform_test() = 0;
		virtual std::string name() const = 0;

		// Get the range of the random variable. This is required to
		// calculate the lil'USB variance parameter in absence of actual
		// variance information.

		virtual double get_minimum() const = 0;
		virtual double get_maximum() const = 0;
};

#endif