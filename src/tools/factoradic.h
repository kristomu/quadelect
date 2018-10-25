#ifndef _TOOLS_FACTORADIC
#define _TOOLS_FACTORADIC

#include "../tools.h"

#include <string>
#include <numeric>

// This class converts between complete permutations (no truncation or
// equal rank) and numbers.

class factoradic {
	public:
		static std::string kth_permutation(int k, int n);
		template<typename T> static uint64_t permutation_number(
			const T & permutation, int n);

		static bool test() {
			return kth_permutation(1, 3) == "ACB" && 
				kth_permutation(2, 3) == "BAC" &&
				permutation_number(kth_permutation(10, 4), 4) == 10;
		}
};

template<typename T> uint64_t factoradic::permutation_number(
	const T & permutation, int n) {

	std::vector<int> identity_perm(n);
	std::iota(identity_perm.begin(), identity_perm.end(), 0);

	int count = n-1;
	uint64_t out = 0;

	for (const char x: permutation) {
		int fac_radix = factorial(count--);
		int element_to_find = x - 'A';
		int element_location = -1;

		for (int j = 0; j < identity_perm.size() && element_location == -1; 
			++j) {

			if (identity_perm[j] == element_to_find) {
				element_location = j;
			}
		}

		if (element_location == -1) {
			throw new std::runtime_error(
				"Can't find element in permutation_number");
		}

		out += element_location * fac_radix;
		identity_perm.erase(identity_perm.begin() + element_location);
	}

	return out;
}

#endif