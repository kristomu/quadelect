#include <stdexcept>
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
		size_t numcands;

	public:
		std::vector<std::vector<size_t> > subelection_members;
		std::vector<std::vector<bool> > is_subelection_member;

		std::vector<size_t> num_options;
		size_t distinct_fingerprints;

		subelections(size_t numcands_in);

		size_t get_num_fingerprint_indices() const;
};

bool subelections::add_subelection(combo::it begin, combo::it end) {
	subelection_members.push_back(std::vector<size_t>(begin, end));
	size_t num_members = end-begin;
	num_options.push_back((1<<num_members) - 2);

	// Also add a vector where is_member[x] is true if the xth
	// candidate is a part of this subelection.
	std::vector<bool> is_member(numcands, false);

	for (combo::it pos = begin; pos != end; ++pos) {
		is_member[*pos] = true;
	}

	return false;
}

subelections::subelections(size_t numcands_in) {

	numcands = numcands_in;

	size_t i;

	// Curry the implicit reference to the calling object.
	auto this_add_subelection = [this](combo::it begin,
	combo::it end) {
		return add_subelection(begin, end);
	};

	for (i = 2; i <= numcands; ++i) {
		// Add every sub-election of i candidates.
		std::vector<size_t> cands(numcands, 0);
		std::iota(cands.begin(), cands.end(), 0);

		for_each_combination(cands.begin(), cands.begin() + i,
			cands.end(), this_add_subelection);
	}

	// Calculate the number of distinct fingerprints
	// based on the number of subelections and the number
	// of ways candidates can be chosen within each.

	distinct_fingerprints = 1;

	for (i = 0; i < num_options.size(); ++i) {
		distinct_fingerprints *= num_options[i];
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

		void decode(const subelections & se, size_t index);
		size_t encode() const;
		void print(const subelections & se) const;
};

void fingerprint::decode(const subelections & se, size_t index) {

	if (index >= se.distinct_fingerprints) {
		throw std::invalid_argument("index exceeds number of fingerprints!");
	}

	for (size_t subelec_idx = 0; subelec_idx < se.num_options.size();
		++subelec_idx) {
		// Get the index to apply to this particular subelection.
		size_t num_options = se.num_options[subelec_idx];
		size_t digit = index % num_options;

		// Then decode it.
		size_t num_members = se.subelection_members[subelec_idx].size();
		std::vector<size_t> candidates_above_quota;

		// Since at least one candidate must exceed the quota,
		// an index of zero, which would correspond to an all-false
		// vector, is disallowed.
		++digit;

		for (size_t member_idx = 0; member_idx < num_members; ++member_idx) {
			if (digit % 2 == 1) {
				candidates_above_quota.push_back(
					se.subelection_members[subelec_idx][member_idx]);
			}
			digit >>= 1;
		}

		above_quota.push_back(candidates_above_quota);

		std::cout << digit << " of " << num_options << "\n";
		index /= num_options;
	}
}

void fingerprint::print(const subelections & se) const {
	size_t i;

	for (size_t subelec_idx = 0; subelec_idx < se.num_options.size();
		++subelec_idx) {

		std::cout << "Subelection {";

		for (i = 0; i < se.subelection_members[subelec_idx].size();
			++i) {
			if (i != 0) {
				std::cout << ", ";
			}
			std::cout << (char)('A' + se.subelection_members[
						subelec_idx][i]);
		}

		std::cout << "}: above quota: ";

		for (i = 0; i < above_quota[subelec_idx].size(); ++i) {
			if (i != 0) {
				std::cout << ", ";
			}

			std::cout << (char)('A' + above_quota[subelec_idx][i]);
		}

		std::cout << "\n";
	}
}

int main() {

	size_t numcands = 4;
	subelections se(numcands);

	fingerprint test;

	std::cout << "Number of distinct fingerprints: "
		<< se.distinct_fingerprints << "\n";

	test.decode(se, 19001);
	test.print(se);

	return 0;
}