#pragma once

#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

typedef ptrdiff_t ssize_t;	/* ssize_t is not part of the C standard */

const double PI = 4 * atan(1);

// Transforms parameters of 0 1 2 ... skip skip+1 ... into
// 0 1 2 ... skip+1 ..., i.e. skips the number given by "skip".
int skip_number(int idx, int skip);

// Sign value: -1 if less than zero, 0 if zero, 1 if greater.
int sign(double in);

// Templated functions must be included in the header so the compiler knows what
// to expand.

template<typename T> T square(const T a) {
	return (a*a);
}

template<typename T> T inc(T in) {
	return (++in);
}

int factorial(int x);

// Create a synthetic candidate name corresponding to the given index, by
// a variant of counting in base 26.
std::string cand_name(ssize_t candidate_index);

// Map inversion - used when parsing ballots, because the main program couldn't
// care less about maps from candidate names to ints, but the printout part
// could use maps from candidate ints to names.

template<typename T, typename Q> std::map<Q, T> invert(
	const std::map<T, Q> & in) {
	std::map<Q, T> inverse_mapping;

	for (typename std::map<T, Q>::const_iterator pos = in.begin();
		pos != in.end(); ++pos) {
		inverse_mapping[pos->second] = pos->first;
	}

	return (inverse_mapping);
}

// Distance on the Lp norm (generalized Euclidean).
double lp_distance(double Lp, const std::vector<double> & a,
	const std::vector<double> & b);

// Integer to string

std::string itos(int source);
std::string lltos(long long source);
std::string dtos(double source);
std::string dtos(double source, double precision);
std::string s_padded(std::string a, size_t maxlen);
std::string s_right_padded(std::string a, size_t maxlen);
std::string itos(int source, unsigned int minlen);
std::string lltos(long long source, unsigned int minlen);

template<typename T> std::string gen_itos(T source) {
	std::ostringstream q;
	q << source;
	return (q.str());
}

// Hexadecimal versions of the above

std::string itos_hex(int source);
std::string lltos_hex(long long source);
std::string itos_hex(int source, unsigned int minlen);
std::string lltos_hex(long long source, unsigned int minlen);

template<typename T> std::string ntos_hex(T source) {
	std::ostringstream q;
	q.flags(std::ios::hex);
	q << source;
	return (q.str());
}

// String to integer.

// For comparison purposes
// (Perhaps include a warning that the number will wrap around?)
// Perhaps also alias both to a common function, to promote code reuse.
// Two levels: strict only returns is_integer on shorts. Normal returns them
// on anything.

template<class T> void arch_stoi(T & dest, bool hex, std::string source) {
	std::stringstream q;
	if (hex) {
		q.flags(std::ios::hex);
	}
	q << source;
	q >> dest;
	return;
}

long long comp_stoi(std::string source);
unsigned int str_toui(std::string source);
int str_toi(std::string source);
double str_tod(std::string source);
uint64_t str_toull(std::string source);
long long comp_stoi_hex(std::string source);
int str_toi_hex(std::string source);
int str_toi_generalized(std::string source);
bool is_integer(std::string source, bool permit_hex);

// Misc string modifications
std::string lowercase(std::string mixed);
std::string uppercase(std::string mixed);
std::string remove_extension(std::string fn);
std::string remove_path(std::string fn);

// Normalization ops.
template<typename T> T norm(T min, T cur, T max) {
	if (max == min) {
		return (0);
	}
	return ((cur-min)/(max-min));
}

template<typename T> T renorm(T min_in, T max_in, T cur, T min_out,
	T max_out) {
	return (norm(min_in, cur, max_in) * (max_out-min_out) + min_out);
}

// Tokenization

std::vector<std::string> tokenize(const std::string & input_string,
	const std::string & delimiters, const char comment_marker,
	bool include_delimiters);

std::string strip_spaces(const std::string & in);
std::vector<std::string> slurp_file(std::ifstream & source,
	bool print_while_slurping);

// Used to read files of the type <number> (space or \t) (other stuff),
// when we're only interested in the numbers.

template <typename T> void get_first_token_on_lines(std::ifstream & source,
	std::vector<T> & out) {

	std::string next;
	T next_integer;

	while (!source.eof()) {
		getline(source, next);
		if (source.eof()) {
			continue;
		}

		std::vector<std::string> tokenized_line = tokenize(
				next, "\t ", '\n', false);
		if (tokenized_line.empty()) {
			continue;
		}
		arch_stoi(next_integer, false, tokenized_line[0]);
		out.push_back(next_integer);
	}
}

// Get the power set of a set of a given cardinality as a vector of
// boolean vectors. The hopefuls are false for indices that should
// never be included.

std::vector<std::vector<bool> > power_set(
	const std::vector<bool> & hopefuls);

std::vector<std::vector<bool> > power_set(int cardinality);

// Round to a specified number of decimals.

double round(double x, double decimals);
