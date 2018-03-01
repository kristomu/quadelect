#include <stdint.h>
#include <iostream>
#include <cstddef>

#include "random.h"

using namespace std;

uint64_t rng::rng64(uint64_t * s) {
        uint64_t c = 7319936632422683419ULL;
        uint64_t x = s[1];

        typedef __uint128_t u128b;
        union u128_64
        {
                uint64_t seed[2];
                u128b val;
        };

        /* Increment 128bit counter */
        ((union u128_64 *)s)->val += c + ((u128b) c << 64);

        /* Two h iterations */
        x ^= (x >> 32);// ^ (u64b) s;
        x *= c;
        x ^= x >> 32;
        x *= c;

        /* Perturb result */
        return x + s[0];
}

void rng::s_rand(int seed_in) {
        seed[0] = 0;
        seed[1] = seed_in;

        int c, d;
        int cmax = 31;
        for (d = 0; d < 3; ++d) {
                for (c = 0; c < cmax; ++c)
                        rng64(seed);

                cmax = 31 + (rng64(seed) + seed[0]) % (63 * (d+1));
                seed[0] += rng64(seed);
        }
}

long double rng::ldrand() {

	// Two results will suffice because in any architecture as of this
	// writing, long double <= 128 bits.

	const uint64_t maxVal = ~((uint64_t)0); // maximum value we'll get.

	const long double RS_SCALE = (1.0 / (1.0 + maxVal));

	long double d;
	do {
		d = ((long_rand() * RS_SCALE) + long_rand()) * RS_SCALE;
	} while (d >= 1);

	return(d);
}

double rng::drand() {
	// Same as above, only double precision. This assumes a double with
	// no more than 64 bits fraction.

	const uint32_t maxVal = ~((uint32_t)0);
	const double RSD_SCALE = (1.0 / (1.0 + maxVal));

	uint64_t baseval;
	uint32_t lower, upper;
	double d;

	do {
		baseval = long_rand();
		lower = baseval;
		upper = baseval >> 32ULL;

		d = ((lower * RSD_SCALE) + upper)*RSD_SCALE;
	} while (d >= 1);

	return(d);
}

double rng::drand(double min, double max) {
	return (min + drand()*(max-min));
}

uint64_t rng::lrand(uint64_t modulus) {
	if (modulus == 0) return(0);

	// Find up to max without succumbing to modulo bias.

	const uint64_t maxVal = ~((uint64_t)0);

	uint64_t remainder = maxVal % modulus;

	// If the modulus evenly divides maxVal, then we're done.
	// Perhaps TODO: Detect if it's a power of two, which should be
	// quicker.
	if (remainder == 0)
		return(long_rand() % modulus);

	// Otherwise, keep trying until we're inside a range that *does*.
	uint64_t randval;

	do
		randval = long_rand();
	while (randval < remainder);

	// And return a modulo on that.
	return((randval - remainder)%modulus);
}

uint64_t rng::lrand(uint64_t min, uint64_t max) {
	if (max < min) return(lrand(max, min));

	return(min + lrand(max-min));
}

uint32_t rng::irand(uint32_t modulus) {
	// Same thing, 32 bit edition.

	if (modulus == 0) return(0);

	const uint32_t maxVal = ~((uint32_t)0);
	uint32_t remainder = maxVal % modulus;

	if (remainder == 0)
		return(irand() % modulus);

	uint32_t randval;
	do
		randval = irand();
	while (randval < remainder);

	return((randval - remainder)%modulus);
}

uint32_t rng::irand(uint32_t min, uint32_t max) {
	if (max < min)
		return(irand(max, min));

	return(min + irand(max-min));
}

/*main() {

	long double accumulated = 0;
	rng myRNG(1);

	for (int counter = 0; counter < 100000; ++counter)
		accumulated += myRNG.drand();

	cout << accumulated << endl;
}*/
