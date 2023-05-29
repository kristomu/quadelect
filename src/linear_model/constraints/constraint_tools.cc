#include "constraint_tools.h"

#include <numeric>
#include <algorithm>

// does A precede B in the permutation?
bool constraint_tools::does_beat(int A, int B,
	const std::vector<int> & permutation) {

	for (int candidate: permutation) {
		if (candidate == A) {
			return true;
		}
		if (candidate == B) {
			return false;
		}
	}

	return false;
}

std::vector<std::vector<int> > constraint_tools::all_permutations(
	int numcands) {

	std::vector<int> perm(numcands);
	std::iota(perm.begin(), perm.end(), 0);

	std::vector<std::vector<int> > permutations;

	do {
		permutations.push_back(perm);
	} while (std::next_permutation(perm.begin(), perm.end()));

	return permutations;
}

// Get the permutations where A beats B
std::vector<std::vector<int> > constraint_tools::get_permutations_beating(
	int A, int B, int numcands) {

	std::vector<int> perm(numcands);
	std::iota(perm.begin(), perm.end(), 0);

	std::vector<std::vector<int> > permutations;

	do {
		if (does_beat(A, B, perm)) {
			permutations.push_back(perm);
		}
	} while (std::next_permutation(perm.begin(), perm.end()));

	return permutations;
}

std::string constraint_tools::permutation_to_str(
	const std::vector<int> & permutation, std::string suffix) {

	std::string out;

	for (int i: permutation) {
		out = out + (char)(i + 'A');
	}

	return out + suffix;
}

relation_side constraint_tools::permutations_to_relation_side(
	const std::vector<std::vector<int> > & permutations,
	std::string prefix, std::string suffix) {

	relation_side out;
	out.constant = 0;

	for (const std::vector<int> & permutation: permutations) {
		std::string var_name = prefix + permutation_to_str(permutation,
				suffix);

		out.weights.push_back(std::pair<std::string, double>(
				var_name, 1));
	}

	return out;
}