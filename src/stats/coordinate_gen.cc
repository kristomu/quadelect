#include "coordinate_gen.h"
#include <stdexcept>

double coordinate_gen::next_double(double min, double max) {
	if (min > max) {
		throw std::invalid_argument("next_double: asking for empty range!");
	}

	return min + next_double() * (max-min);
}

uint32_t coordinate_gen::next_int(uint32_t begin, uint32_t end) {
	if (begin >= end) {
		throw std::invalid_argument("next_int: asking for empty range!");
	}

	return begin + next_int(end-begin);
}

uint64_t coordinate_gen::next_long(uint64_t begin, uint64_t end) {
	if (begin >= end) {
		throw std::invalid_argument("next_long: asking for empty range!");
	}

	return begin + next_long(end-begin);
}