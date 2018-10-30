#ifndef __TOOLS
#define __TOOLS

#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <assert.h>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>

const double PI = 4 * atan(1);

// Templated functions must be included in the header so the compiler knows what
// to expand.

template<typename T> T square(const T a) {
	return (a*a);
}

template<typename T> T inc(T in) {
	return (++in);
}

int factorial(int x);

// Map inversion - used when parsing ballots, because the main program couldn't
// care less about maps from candidate names to ints, but the printout part
// could use maps from candidate ints to names.

template<typename T, typename Q> std::map<Q, T> invert(const std::map<T, Q> & in) {
	std::map<Q, T> inverse_mapping;

	for (typename std::map<T, Q>::const_iterator pos = in.begin();
	    pos != in.end(); ++pos)
		inverse_mapping[pos->second] = pos->first;

	return (inverse_mapping);
}

double get_abs_time();
// Distance on the Lp norm (generalized Euclidean).
double euc_distance(double Lp, const std::vector<double> & a,
    const std::vector<double> & b);

// Integer to string

std::string itos (int source);
std::string lltos(long long source);
std::string dtos (double source);
std::string dtos (double source, double precision);
std::string s_padded(std::string a, size_t maxlen);
std::string s_right_padded(std::string a, size_t maxlen);
std::string itos(int source, unsigned int minlen);
std::string lltos(long long source, unsigned int minlen);

// Hexadecimal versions of the above

std::string itos_hex(int source);
std::string lltos_hex(long long source);
std::string itos_hex(int source, unsigned int minlen);
std::string lltos_hex(long long source, unsigned int minlen);

// String to integer.

// For comparison purposes
// (Perhaps include a warning that the number will wrap around?)
// Perhaps also alias both to a common function, to promote code reuse.
// Two levels: strict only returns is_integer on shorts. Normal returns them
// on anything.

template<class T> void arch_stoi(T & dest, bool hex, std::string source) {
	std::stringstream q;
	if (hex)
		q.flags(std::ios::hex);
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
	if (max == min) return (0);
	return ((cur-min)/(max-min));
}

template<typename T> T renorm(T min_in, T max_in, T cur, T min_out, T max_out) {
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

std::vector<std::string> get_first_token_on_lines(std::ifstream & source);

#endif
