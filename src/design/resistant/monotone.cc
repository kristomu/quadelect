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
		// "members" are indexed by the candidates present *in that
		// subelection*. On the other hand, "candidates" refer to the
		// total set of candidates for the election as a whole.

		// For each subelection, a boolean vector of whether the
		// kth member has exceeded the quota.
		std::vector<std::vector<bool> > is_member_above_quota;

		// Translation function for turning a boolean vector
		// into a vector of candidates above the quota.
		// Used for printing.
		std::vector<size_t> get_candidates_above_quota(
			const subelections & se, size_t subelection_idx,
			const std::vector<bool> & above_quota_here) const;

		void decode(const subelections & se, size_t index);
		size_t encode(const subelections & se) const;
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

		std::cout << "index is " << index << " and digit is " << digit <<
			std::endl;
		std::cout << "num options: " << num_options << "\n";

		// Then decode it.
		size_t num_members = se.subelection_members[subelec_idx].size();
		std::vector<bool> above_quota_here;

		// Since at least one candidate must exceed the quota,
		// an index of zero, which would correspond to an all-false
		// vector, is disallowed.
		++digit;

		for (size_t member_idx = 0; member_idx < num_members; ++member_idx) {
			if (digit % 2 == 1) {
				above_quota_here.push_back(true);
			} else {
				above_quota_here.push_back(false);
			}
			digit >>= 1;
		}

		is_member_above_quota.push_back(above_quota_here);

		std::cout << digit << " of " << num_options << "\n";
		index /= num_options;
	}
}

std::vector<size_t> fingerprint::get_candidates_above_quota(
	const subelections & se, size_t subelection_idx,
	const std::vector<bool> & above_quota_here) const {

	std::vector<size_t> candidates_above_quota;
	size_t num_members = se.subelection_members[subelection_idx].size();

	for (size_t member_idx = 0; member_idx < num_members; ++member_idx) {
		if (above_quota_here[member_idx]) {
			candidates_above_quota.push_back(
				se.subelection_members[subelection_idx][member_idx]);
		}
	}

	return candidates_above_quota;
}

size_t fingerprint::encode(const subelections & se) const {

	size_t output_index = 0;

	// Get the "places" ("tens", "ones", etc) for the
	// mixed radix number system.
	std::vector<size_t> places;
	size_t running_count = 1;
	for (size_t i = 0; i < se.num_options.size(); ++i) {
		places.push_back(running_count);
		running_count *= se.num_options[i];
	}

	for (size_t i = 0; i < se.num_options.size(); ++i) {

		// We need to go from the last subelection to get the
		// digits in the proper order.
		size_t subelec_idx = i;

		// Get the contribution to the output index by the chosen
		// candidates for this subelection.

		size_t digit = 0, range = 0;

		for (size_t j = 0; j < is_member_above_quota[subelec_idx].size();
			++j) {

			if (is_member_above_quota[subelec_idx][j]) {
				digit += (1<<j);
			}
		}

		range = (1<<is_member_above_quota[subelec_idx].size())-1;

		if (digit == 0) {
			throw std::logic_error("Bug: no candidate above quota"
				" in subelection???");
		}
		if (digit == range) {
			throw std::logic_error("Bug: every candidate above quota"
				" in subelection???");
		}

		--digit;

		std::cout << "index is " << output_index << " and digit is " << digit <<
			std::endl;
		std::cout << "range is " << range << "\n";

		output_index += digit * places[i];
	}

	return output_index;

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

		std::vector<size_t> cands_above_quota =
			get_candidates_above_quota(se, subelec_idx,
				is_member_above_quota[subelec_idx]);

		for (i = 0; i < cands_above_quota.size(); ++i) {
			if (i != 0) {
				std::cout << ", ";
			}

			std::cout << (char)('A' + cands_above_quota[i]);
		}

		std::cout << "\n";
	}
}

// TODO: encode()
// TODO: monotonicity. Raising A can stop B from being above the quota in
// any subelection where A and B are both present. It doesn't do anything
// in subelections not having A. It may also (irrespective of the above)
// make A pass the quota in some subelection he didn't have it before.
// Probably devise a function that lists every subelection change that
// may occur; then a recursive function that creates the n-ary Cartesian
// product of these. (That might be a lot of edges...)

int main() {

	size_t numcands = 4;
	subelections se(numcands);

	fingerprint test;

	std::cout << "Number of distinct fingerprints: "
		<< se.distinct_fingerprints << "\n";

	test.decode(se, 19001);
	test.print(se);

	std::cout << test.encode(se) << "\n";

	return 0;
}