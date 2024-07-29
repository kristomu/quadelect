// Custom random number generator. On an x64 architecture, it's faster than
// random(), and it passes Crush even when the seed is quickly altered, making
// it useful for per-case randomization.

// The reason this does not have a () constructor that seeds from the entropy
// source is so that variable declarations like rng whatever; fail instead of
// silently being set. This helps control randomness and keeps things
// reproducible.

#pragma once

#include <stdint.h>
#include <iostream>
#include <cstddef>

#include "../stats/coordinate_gen.h"

#define rseed_t uint64_t
// Set seed to this to draw seed from entropy source.
#define RNG_ENTROPY 0

class rng : public coordinate_gen {
	private:
		uint64_t initial_seed;
		uint64_t seed[2];				// Actually the RNG's state
		uint64_t rng64(uint64_t * s);

	public:

		// Some boilerplate duie to coordinate_gen
		bool is_independent() const {
			return true;
		}

		// Perhaps we should enforce that start_query needs to be
		// called once before the RNG is ever used... and perhaps
		// count the number of variates generated between queries
		// for debug purposes? TODO???
		void start_query() {}
		void end_query() {}

		//typedef uint64_t result_type;

		void s_rand(uint64_t seed_in); // Now passes Crush (not BigCrush)
		void s_rand();                 // from entropy source

		uint64_t get_initial_seed() const;

		rng(uint64_t seed_in) {
			s_rand(seed_in);
		}

		using coordinate_gen::next_long;
		using coordinate_gen::next_int;
		using coordinate_gen::next_double;

		uint64_t next_long() {
			return rng64(seed);
		}

		uint32_t next_int() {
			return next_long();
		}
		long double ldrand();
		double next_double();

		// Ranges. Note that these are all half-open, i.e. [0 ... modulus)
		uint64_t next_long(uint64_t modulus);
		uint32_t next_int(uint32_t modulus);

		std::vector<double> get_coordinate(size_t dimension);
};