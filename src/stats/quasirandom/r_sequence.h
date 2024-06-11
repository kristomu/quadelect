#pragma once

// For generating a d-dimensional R_d quasirandom sequence, as given by
// https://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
// archived link with pictures intact: https://web.archive.org/web/20230228120508/https://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

// This can be used for better Monte-Carlo sampling, and thus should be
// useful both for Bayesian regret/VSE and Yee maps.

// Note that a Sobol sequence may be better, but this sequence is much easier
// to implement.

#include <vector>
#include <stdexcept>
#include "../coordinate_gen.h"

const size_t R_SEQ_NOT_INITED = (size_t) -1;

class r_sequence : public coordinate_gen {
	private:
		std::vector<double> alpha_roots;
		std::vector<double> current_state, current_point;

		void set_alpha_roots(size_t dimension);

		size_t query_pos = R_SEQ_NOT_INITED;

	public:
		r_sequence(size_t dimension) {
			current_state = std::vector<double>(dimension, 0);
			current_point = current_state;
			set_alpha_roots(dimension);

			// We don't want to return (0.5, .., 0.5) so advance
			// the state by one.
			next();
		}

		bool is_independent() const {
			return false;
		}
		void start_query();

		void end_query();

		using coordinate_gen::next_long;
		using coordinate_gen::next_int;
		using coordinate_gen::next_double;

		double next_double();

		uint64_t next_long();
		uint64_t next_long(uint64_t modulus);

		uint32_t next_int();
		uint32_t next_int(uint32_t modulus);

		// Maybe I could make this into an iterator... :-P

		// TODO: Seeding (skip to a certain point) so we can
		// make it at least a *bit* random for repeated trials/
		// hybrid Monte Carlo.

		std::vector<double> cur() const {
			return current_point;
		}

		std::vector<double> next();

		std::vector<double> get_coordinate(size_t dimension) {
			if (dimension != current_state.size()) {
				throw std::invalid_argument("get_coordinate: Dimension mismatch");
			}

			return next();
		}
};
