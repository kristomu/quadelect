#pragma once

// The combination_generator generates subsets of of the range 0...n.
// This should be easier to deal with, though strictly speaking somewhat slower,
// than the library I was using earlier.

// I can always generalize later if I need arbitrary set support. YAGNI, etc.

#include <vector>
#include <stdexcept>
#include <algorithm>

class combination_generator {
	private:
		std::vector<size_t> subset;
		std::vector<bool> is_chosen;
		size_t k, n;

		// Update the subset vector to reflect the boolean state.
		void update_subset() {
			size_t subset_idx = 0;

			for (size_t i = 0; i < n; ++i) {
				if (is_chosen[i]) {
					subset[subset_idx++] = i;
				}
			}
		}

	public:
		bool next() {
			// Transition to the next permutation of the boolean present/not
			// variables if possible. If it was possible, update the
			// actual subset numbers to match.
			if (std::prev_permutation(is_chosen.begin(), is_chosen.end())) {
				update_subset();
				return true;
			}
			return false;
		}

		combination_generator(size_t n_in, size_t k_in) {
			// TODO: more error checking?
			k = k_in;
			n = n_in;

			if (k > n) {
				throw std::domain_error("combination_generator: "
					"Subset can't be larger than set it's drawn from.");
			}

			is_chosen.resize(n);
			subset.resize(k);
			std::fill(is_chosen.begin(), is_chosen.begin() + k, true);

			// Update the subset to reflect the starting combination.
			update_subset();
		}

		std::vector<size_t>::const_iterator begin() {
			return subset.begin();
		}

		std::vector<size_t>::const_iterator end() {
			return subset.end();
		}
};