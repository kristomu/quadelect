#include <iostream>
#include <numeric>
#include <vector>

#include "lib/combinations/combinations.h"

// First define the resistant set "fingerprint" classes.

// A fingerprint is consists of a number of binary vectors, one for each
// sub-election, where a sub-election's binary vector designates the
// candidates whose first preference counts are above the "quota" of
// 1/(num candidates in the sub-election).

// An election of n candidates has 2^n - (n+1) sub-elections: all binary
// representations of n bits, apart from n one-bit values (which would
// be sub-elections with a single candidate in it) and one zero-bit value
// that has no candidates.

// Each subelection of k candidates's fingerprint contribution can be
// set to one of 2^k - 2 values: all options except all true and all false.
// At least one candidate must exceed the quota, but they can't all do so
// simultaneously. This thus lets us map each fingerprint to an integer
// by using a mixed radix number system.

// To translate between numbers and actual sub-elections, an auxiliary
// class gives the subelection subsets and maximum value for each. Define
// this first.

namespace combo {
typedef std::vector<size_t>::const_iterator it;
}

class subelections {
	private:
		bool add_subelection(combo::it begin, combo::it end);

	public:
		std::vector<std::vector<size_t> > subelection_members;
		std::vector<size_t> max_options;
		subelections(size_t numcands);

		size_t get_num_fingerprint_indices() const;
};

bool subelections::add_subelection(combo::it begin, combo::it end) {
	subelection_members.push_back(std::vector<size_t>(begin, end));
	size_t num_members = end-begin;
	max_options.push_back((1<<num_members) - 2);

	return false;
}

subelections::subelections(size_t numcands) {

	// Curry the implicit reference to the calling object.
	auto this_add_subelection = [this](combo::it begin,
	combo::it end) {
		return add_subelection(begin, end);
	};

	for (size_t i = 2; i <= numcands; ++i) {
		// Add every sub-election of i candidates.
		std::vector<size_t> cands(numcands, 0);
		std::iota(cands.begin(), cands.end(), 0);

		for_each_combination(cands.begin(), cands.begin() + i,
			cands.end(), this_add_subelection);
	}
}


// The decoded version is represented as a vector of chosen candidates
// for each sub-election. The sub-election in question can be determined
// by its index.

// above_quota[i] contains j if candidate j is above the 1/k quota of
// first preferences in the ith subelection.

class fingerprint {
	public:
		std::vector<std::vector<size_t> > above_quota;

		void decode(size_t index);
		size_t encode() const;
};

int main() {

	size_t numcands = 3;

	subelections se(numcands);

	return 0;
}