#pragma once

#include <vector>

#include "test_generator_group.h"

class test_generator_groups {
	public:
		// Could be made into a map if I implement operator< on the
		// scenario tuples, but that's not yet necessary.
		std::vector<test_generator_group> groups;

		void insert(test_instance_generator candidate) {
			for (test_generator_group & grp: groups) {
				if (grp.fits_group(candidate)) {
					grp.insert(candidate);
					return;
				}
			}
			groups.push_back(test_generator_group());
			groups.rbegin()->insert(candidate); // will always succeed
		}
};