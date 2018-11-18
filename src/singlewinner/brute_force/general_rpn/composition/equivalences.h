#pragma once

// After permuting according to cand_permutation, we end up in
// to_scenario.

struct isomorphism {
	std::vector<std::vector<int> > cand_permutations;
	copeland_scenario to_scenario;
	bool derived;
};