#include "r_sequence.h"

#include <stdexcept>
#include <cmath>

void r_sequence::set_alpha_roots(size_t dimension) {
	double g = 2;

	// We probably need much fewer than 30 iterations to
	// reach convergence, but hey, we can afford the CPU
	// cycles.
	for (size_t iter = 0; iter < 30; ++iter) {
		g = pow(1+g, 1.0/(dimension+1));
	}

	for (size_t dim = 1; dim <= dimension; ++dim) {
		alpha_roots.push_back(1.0/pow(g, dim));
	}
}

std::vector<double> r_sequence::next() {
	if (query_pos != current_state.size() && query_pos != R_SEQ_NOT_INITED) {
		throw std::logic_error(
			"Tried to create new sequence without consuming all coordinate points!");
	}

	for (size_t i = 0; i < current_point.size(); ++i) {
		current_state[i] = fmod(current_state[i]+alpha_roots[i], 1);

		// Add 0.5 as recommended in the article.
		current_point[i] = fmod(current_state[i] + 0.5, 1);
	}

	return current_point;
}

void r_sequence::start_query() {
	if (query_pos != current_state.size() && query_pos != R_SEQ_NOT_INITED) {
		throw std::logic_error(
			"Tried to start query without consuming all coordinate points!");
	}
	next();
	query_pos = 0;
}

void r_sequence::end_query() {
	if (query_pos != current_state.size()) {
		throw std::logic_error(
			"Ended query without having used all coordinate points!");
	}
}

double r_sequence::next_double() {
	if (query_pos == current_state.size()) {
		throw std::logic_error("double supply exhausted");
	}
	if (query_pos == R_SEQ_NOT_INITED) {
		throw std::logic_error("next_double without start_query");
	}
	return current_state[query_pos++];
}

uint64_t r_sequence::next_long() {
	// Maybe not such a good idea due to roundoff problems
	// TODO: Do something better
	// Maybe https://stackoverflow.com/questions/65552315/ ???
	return round(next_double() * UINT64_MAX);
}

uint64_t r_sequence::next_long(uint64_t modulus) {
	return round(next_double() * (modulus-1));
}

uint32_t r_sequence::next_int() {
	return round(next_double() * UINT32_MAX);
}

uint32_t r_sequence::next_int(uint32_t modulus) {
	return round(next_double() * (modulus-1));
}