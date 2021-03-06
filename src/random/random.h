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

#define rseed_t uint64_t
// Set seed to this to draw seed from entropy source.
#define RNG_ENTROPY 0

class rng {
	private:
		uint64_t seed[2];
		uint64_t rng64(uint64_t * s);

	public:
		void s_rand(uint64_t seed_in); // Now passes Crush (not BigCrush)
		void s_rand();                 // from entropy source

		rng(uint64_t seed_in) {
			s_rand(seed_in);
		}

		uint64_t long_rand() {
			return (rng64(seed));
		}

		uint32_t irand() {
			return (long_rand());
		}
		long double ldrand();
		double drand();
		double drand(double min, double max);

		// Ranges. Note that these are all half-open, i.e. [min, max)
		uint64_t lrand(uint64_t modulus);
		uint64_t lrand(uint64_t min, uint64_t max);

		uint32_t irand(uint32_t modulus);
		uint32_t irand(uint32_t min, uint32_t max);

		// Used for random_shuffle etc. Assumes pointers are no longer
		// than 64 bit. Perhaps using () is a bit of a hack, but the
		// alternatives are worse.
		ptrdiff_t operator()(ptrdiff_t max) {
			return (lrand(max));
		}
};