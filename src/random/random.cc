// Now uses xorshift128+, seeded by xorshift64*.

#include <stdexcept>
#include <iostream>
#include <cstddef>
#include <random>

#include <stdint.h>
#include "random.h"

using namespace std;

uint64_t rng::rng64(uint64_t * s) {
	uint64_t s1 = s[ 0 ];
	const uint64_t s0 = s[ 1 ];
	s[ 0 ] = s0;
	s1 ^= s1 << 23; // a
	return (s[ 1 ] = (s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26))) + s0;
}

// Clock one iteration of xorshift64* to improve Crush results with
// rapidly changing seeds.
void rng::s_rand(uint64_t seed_in) {
	initial_seed = seed_in;
	seed[0] = 0;

	// We can't ordinarily seed xorshift with all zeroes, so if we're
	// given a zero, then set get the seed_in from an entropy source
	// instead.
	if (seed_in == RNG_ENTROPY) {
		s_rand();
		return;
	}

	if (seed_in == 0) {
		throw std::logic_error("RNG: Seed is zero! (This shouldn't happen)");
	}

	// Xorshift64*
	seed_in ^= seed_in >> 12; // a
	seed_in ^= seed_in << 25; // b
	seed_in ^= seed_in >> 27; // c
	seed[1] = seed_in * 2685821657736338717LL;
}

void rng::s_rand() {
	// std::random_device is a C++ hardware RNG class. It returns
	// unsigned 32-bit ints, so we need two of them.
	std::random_device rd;
	uint64_t seed_in = 0;

	// Guard against a 2^-64 probability event and platforms where
	// rd is not random at all, but returns 0 all the time.
	for (int i = 0; i < 10 && seed_in == 0; ++i) {
		uint64_t a = rd(), b = rd();
		seed_in = (a << 32ULL) + b;
	}

	if (seed_in == 0) {
		throw std::runtime_error(
			"RNG: std::random_device always returns 0");
	}

	s_rand(seed_in);
}

uint64_t rng::get_initial_seed() const {
	return initial_seed;
}

long double rng::ldrand() {

	// Two results will suffice because in any architecture as of this
	// writing, long double <= 128 bits.

	const uint64_t maxVal = ~((uint64_t)0); // maximum value we'll get.

	const long double RS_SCALE = (1.0 / (1.0 + (long double)maxVal));

	long double d;
	do {
		d = ((next_long() * RS_SCALE) + next_long()) * RS_SCALE;
	} while (d >= 1);

	return (d);
}

double rng::next_double() {

	// Same as above, only double precision. This assumes a double with
	// no more than 64 bits fraction.

	const uint32_t maxVal = ~((uint32_t)0);
	const double RSD_SCALE = (1.0 / (1.0 + maxVal));

	uint64_t baseval;
	uint32_t lower, upper;
	double d;

	do {
		baseval = next_long();
		lower = baseval;
		upper = baseval >> 32ULL;

		d = ((lower * RSD_SCALE) + upper)*RSD_SCALE;
	} while (d >= 1);

	return (d);
}

uint64_t rng::next_long(uint64_t modulus) {
	if (modulus == 0) {
		return (0);
	}

	// Find up to max without succumbing to modulo bias.

	const uint64_t maxVal = ~((uint64_t)0);

	uint64_t remainder = maxVal % modulus;

	// If the modulus evenly divides maxVal, then we're done.
	if (remainder == 0) {
		return (next_long() % modulus);
	}

	// Otherwise, keep trying until we're inside a range that *does*.
	uint64_t randval;

	do {
		randval = next_long();
	} while (randval < remainder);

	// And return a modulo on that.
	return ((randval - remainder)%modulus);
}

uint32_t rng::next_int(uint32_t modulus) {
	// Same thing, 32 bit edition.

	if (modulus == 0) {
		return (0);
	}

	const uint32_t maxVal = ~((uint32_t)0);
	uint32_t remainder = maxVal % modulus;

	if (remainder == 0) {
		return (next_int() % modulus);
	}

	uint32_t randval;
	do {
		randval = next_int();
	} while (randval < remainder);

	return ((randval - remainder)%modulus);
}

std::vector<double> rng::get_coordinate(size_t dimension) {
	std::vector<double> out(dimension);

	for (size_t i = 0; i < dimension; ++i) {
		out[i] = next_double();
	}

	return out;
}

/*main() {

	long double accumulated = 0;
	rng myRNG(1);

	for (int counter = 0; counter < 100000; ++counter)
		accumulated += myRNG.next_double();

	cout << accumulated << endl;
}*/
