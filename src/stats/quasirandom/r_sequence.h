#pragma once

// For generating a d-dimensional R_d quasirandom sequence, as given by
// https://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

// This can be used for better Monte-Carlo sampling, and thus should be
// useful both for Bayesian regret/VSE and Yee maps.

// Note that a Sobol sequence may be better, but this sequence is much easier
// to program.

#include <vector>

class r_sequence {
	private:
		std::vector<double> alpha_roots;
		std::vector<double> current_state, current_point;

		void set_alpha_roots(size_t dimension);

	public:
		r_sequence(size_t dimension) {
			current_state = std::vector<double>(dimension, 0);
			current_point = current_state;
			set_alpha_roots(dimension);

			// We don't want to return (0.5, .., 0.5) so advance
			// the state by one.
			next();
		}

		// Maybe I could make this into an iterator... :-P

		std::vector<double> cur() const {
			return current_point;
		}

		std::vector<double> next();
};