#include "factoradic.h"
//#include <iostream>

// Find the kth permutation of numcands candidates using factorial base
// logic. E.g. the 0th for 3 candidates is ABC and the 5th is CBA.
std::string factoradic::kth_permutation(int k, int numcands) {

	std::vector<char> candidates(numcands);
	std::iota(candidates.begin(), candidates.end(), 'A');

	std::string output = "";

	for (int radix = numcands-1; radix >= 0; --radix) {
		// Needs to be done MSB style for some reason.
		int digit = k / factorial(radix);
		k -= factorial(radix) * digit;

		output += candidates[digit];
		candidates.erase(candidates.begin()+digit);
	}

	return (output);
}

/*main() {
	if (factoradic().test()) {
		std::cout << "OK" << std::endl;
	}
}*/