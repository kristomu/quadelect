// Now uses xorshift128+, seeded by xorshift64*.

#include <stdint.h>
#include <iostream>
#include <cstddef>

#include <sys/random.h>

#include "random.h"
#include "../tools/tools.h"

using namespace std;

uint64_t rng::rng64(uint64_t * s) {
	uint64_t s1 = s[ 0 ];
	const uint64_t s0 = s[ 1 ];
	s[ 0 ] = s0;
	s1 ^= s1 << 23; // a
	return ( s[ 1 ] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0;
}

// Clock one iteration of xorshift64* to improve Crush results with
// rapidly changing seeds.
void rng::s_rand(uint64_t seed_in) {
	seed[0] = 0;

	// We can't ordinarily seed xorshift with all zeroes, so if we're
	// given a zero, then set get the seed_in from an entropy source
	// instead.
	if (seed_in == RNG_ENTROPY)
		s_rand();

	// Xorshift64*
	seed_in ^= seed_in >> 12; // a
	seed_in ^= seed_in << 25; // b
	seed_in ^= seed_in >> 27; // c
	seed[1] = seed_in * 2685821657736338717LL;
}

void rng::s_rand() {
	uint64_t seed_in[1];
	ssize_t numbytes = getrandom(seed_in, sizeof(uint64_t), 0);

	if (numbytes != sizeof(uint64_t)) {
		throw std::runtime_error("rng: could only read " + itos(numbytes) +
			" bytes from entropy for seeding.");
	}
	s_rand(seed_in[0]);
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

	return (d);
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

	return (d);
}

double rng::drand(double min, double max) {
	return (min + drand()*(max-min));
}

uint64_t rng::lrand(uint64_t modulus) {
	if (modulus == 0) return (0);

	// Find up to max without succumbing to modulo bias.

	const uint64_t maxVal = ~((uint64_t)0);

	uint64_t remainder = maxVal % modulus;

	// If the modulus evenly divides maxVal, then we're done.
	if (remainder == 0)
		return (long_rand() % modulus);

	// Otherwise, keep trying until we're inside a range that *does*.
	uint64_t randval;

	do {
		randval = long_rand();
	} while (randval < remainder);

	// And return a modulo on that.
	return ((randval - remainder)%modulus);
}

uint64_t rng::lrand(uint64_t min, uint64_t max) {
	if (max < min) return (lrand(max, min));

	return (min + lrand(max-min));
}

uint32_t rng::irand(uint32_t modulus) {
	// Same thing, 32 bit edition.

	if (modulus == 0) return (0);

	const uint32_t maxVal = ~((uint32_t)0);
	uint32_t remainder = maxVal % modulus;

	if (remainder == 0)
		return (irand() % modulus);

	uint32_t randval;
	do
		randval = irand();
	while (randval < remainder);

	return ((randval - remainder)%modulus);
}

uint32_t rng::irand(uint32_t min, uint32_t max) {
	if (max < min)
		return (irand(max, min));

	return (min + irand(max-min));
}

/*main() {

	long double accumulated = 0;
	rng myRNG(1);

	for (int counter = 0; counter < 100000; ++counter)
		accumulated += myRNG.drand();

	cout << accumulated << endl;
}*/
