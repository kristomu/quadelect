#pragma once

#include "../test_generator.h"
#include "../vector_ballot.h"
#include <vector>

// This is a list of results gathered by testing multiple candidate
// algorithms against a particular test generator or test generator group.
// As long as the scenarios are the same for each test instance generator,
// they can be treated as the same test instance generator as far as the
// test results are concerned.

class vector_test_instance {
	public:
		std::vector<std::vector<double> > ballot_vectors;
		relative_test_instance ti;

		vector_test_instance(const relative_test_instance in) {
			ti = in;
			// Convert the test instance to ballot vectors.
			ballot_vectors = {
				get_ballot_vector(ti.before_A),
				get_ballot_vector(ti.before_B),
				get_ballot_vector(ti.after_A),
				get_ballot_vector(ti.after_B)
			};
		}
};