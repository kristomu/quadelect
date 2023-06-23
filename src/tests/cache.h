#pragma once

// The way the test runner works, it doesn't have any information about
// what kind of criteria are being tested, due to limitations of static
// typing. However, the tests often require access to some kind of data
// inferred from an election to create variants of that election to see
// if the criterion can be failed. It's too expensive to rederive these
// every time, so they need to be cached.

// Since the cache can't know what kind of criteria will be tested, the
// cache object will have to contain every possible derived data type,
// even if only some of them will be used at any time. Thus the cache
// needs to include *every* such class definition, which makes it rather
// ugly. But this is the best solution I could come up with.

#include "strategy/ballots_by_support.h"

// XXX: This doesn't yet contain a hash that will identify the election it's
// valid for. It must thus never be used across elections. Implementing a
// hash function for elections would fix that problem, but I don't know
// if it's worth it, performance wise.
class test_cache {
	public:
		std::vector<ballots_by_support> grouped_by_challenger;
};