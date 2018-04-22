#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>

#include "tools.h"

double get_abs_time() {
	timeval tv;
	if (gettimeofday(&tv, NULL) == 0)
		return (tv.tv_sec + tv.tv_usec/1e6);
	else    return (0);
}

// Distance on the Lp norm (generalized Euclidean).
double euc_distance(double Lp, const std::vector<double> & a,
                    const std::vector<double> & b) {

	double sum = 0;
	size_t num_elements = std::min(a.size(), b.size());

	for (size_t counter = 0; counter < num_elements; ++counter)
		sum += fabs(pow(a[counter]-b[counter], Lp));

	return (pow(sum/(double)num_elements, 1.0/Lp));
}

// Integer to string

std::string itos (int source) {
	std::ostringstream q;
	q << source;
	return (q.str());
}

std::string lltos(long long source) {
	std::ostringstream q;
	q << source;
	return (q.str());
}

std::string dtos (double source) {
	std::ostringstream q;
	q << source;
	return (q.str());
}

std::string dtos (double source, double precision) {
	return (dtos(round(source * pow(10.0, precision)) /
	             pow(10.0, precision)));
}

std::string s_padded(std::string a, size_t maxlen) {
	if (maxlen < a.size()) return a;

	size_t len = std::max((size_t)0, maxlen - a.size());

	return (a + std::string(len, ' '));
}

std::string s_right_padded(std::string a, size_t maxlen) {
	if (maxlen < a.size()) return a;

	size_t len = std::max((size_t)0, maxlen - a.size());

	return (std::string(len, ' ') + a);
}

// Integer to string, padded to size.
std::string itos(int source, unsigned int minlen) {
	std::string basis = itos(source);
	if (basis.size() < minlen) {
		std::string q(minlen - basis.size(), '0');
		return (q + basis);
	} else
		return (basis);
}

std::string lltos(long long source, unsigned int minlen) {
	std::string basis = lltos(source);
	if (basis.size() < minlen) {
		std::string q(minlen - basis.size(), '0');
		return (q + basis);
	} else
		return (basis);
}

// Hexadecimal versions of the above

std::string itos_hex(int source) {
	std::ostringstream q;
	q.flags(std::ios::hex);
	q << source;
	return (q.str());
}

std::string lltos_hex(long long source) {
	std::ostringstream q;
	q.flags(std::ios::hex);
	q << source;
	return (q.str());
}

std::string itos_hex(int source, unsigned int minlen) {
	std::string basis = itos_hex(source);
	if (basis.size() < minlen) {
		std::string q(minlen - basis.size(), '0');
		return (q + basis);
	} else
		return (basis);
}

std::string lltos_hex(long long source, unsigned int minlen) {
	std::string basis = lltos_hex(source);
	if (basis.size() < minlen) {
		std::string q(minlen - basis.size(), '0');
		return (q + basis);
	} else
		return (basis);
}

// String to integer.

// For comparison purposes
// (Perhaps include a warning that the number will wrap around?)
// Perhaps also alias both to a common function, to promote code reuse.
// Two levels: strict only returns is_integer on shorts. Normal returns them
// on anything.

// arch_stoi in header.

long long comp_stoi(std::string source) {
	long long output = 0; // In case source is empty
	arch_stoi(output, false, source);
	return (output);
}

unsigned int str_toui(std::string source) {
	unsigned int output = 0;
	arch_stoi(output, false, source);
	return (output);
}

double str_tod(std::string source) {
	double output = 0;
	arch_stoi(output, false, source);
	return (output);
}

uint64_t str_toull(std::string source) {
	uint64_t output = 0;
	arch_stoi(output, false, source);
	return (output);
}

int str_toi(std::string source) {
	int output = 0;
	arch_stoi(output, false, source);
	return (output);
}

long long comp_stoi_hex(std::string source) {
	long long output = 0;
	arch_stoi(output, true, source);
	return (output);
}

int str_toi_hex(std::string source) {
	int output = 0;
	arch_stoi(output, true, source);
	return (output);
}

int str_toi_generalized(std::string source) {
	// If it's too short to have the hex qualifiers, go right to stoi
	if (source.size() < 2) return (str_toi(source));
	// Okay, test if it's hex. If so, strip off the qualifier and return
	// stoi_hex for the integer.
	if (source.size() > 2 && source[0] == '0' && source[1] == 'x')
		return (str_toi_hex(source.substr(2, source.size() - 2)));
	if (*(source.end()-1) == 'h')
		return (str_toi_hex(source.substr(0, source.size() - 1)));

	return (str_toi(source));
}

bool is_integer(std::string source, bool permit_hex) {
	// DEBUG
	//std::cout << "Checking is_integer: [" << source << "]" << std::endl;
	// First, find out if it's hex and strip if so.
	bool is_hex = false;

	if (source.size() >= 2) {
		if (source.size() > 2 && source[0] == '0' &&source[1] == 'x') {
			is_hex = true;
			source = source.substr(2, source.size() - 2);
		}
		if (*(source.end()-1) == 'h') {
			is_hex = !is_hex;
			source = source.substr(0, source.size() - 1);
		}
	}

	// Special case of -0. We permit this because of precedent; quite a
	// number of robots use this (perhaps because of rounding from nonints).
	if (source == "-0")
		source = "0";

	// We handle leading zeroes by expanding the itos to the right size.
	if (is_hex && permit_hex)
		return (source == itos_hex(comp_stoi_hex(source),
		                           source.size()));
	else	return (source == itos(comp_stoi(source), source.size()));
}

// Misc string modifications
std::string lowercase(std::string mixed) {
	std::string toRet = mixed;
	transform(toRet.begin(), toRet.end(), toRet.begin(),
	          (int(*)(int)) tolower); // Isn't this funny?
	return (toRet);
}

std::string uppercase(std::string mixed) {
	std::string toRet = mixed;
	transform(toRet.begin(), toRet.end(), toRet.begin(),
	          (int(*)(int)) toupper);	// Second verse..
	return (toRet);
}

std::string remove_extension(std::string fn) {
	// Just search from the end and then chop off at point of first .
	// if any, otherwise entire string.
	// Might fail if there are no extensions but path has . somewhere.
	// 	(Can be "solved" by breaking at first /, then causes problems
	// 	 with filenames with \/ in their extensions.)
	// Fix later. And you could probably use some sort of nifty STL
	// trick to do this without a loop.

	bool found = false;
	int pos = -1;
	// > 0 because a file that starts with a period shouldn't have that
	// period counted as the start of an extension.
	for (int counter = fn.size()-1; counter > 0 && !found; --counter) {
		if (fn[counter] == '.') {
			found = true;
			pos = counter;
		}
	}

	if (found)
		return (fn.substr(0, pos));
	else	return (fn);
}

std::string remove_path(std::string fn) {

	// Same as r_e, only that it turns pathnames into filenames by cutting
	// away /.

	bool found = false;
	int pos = -1;

	for (int counter = fn.size()-1; counter > 0 && !found; --counter) {
		if (fn[counter] == '/') {
			found = true;
			pos = counter;
		}
	}

	if (found)
		return (fn.substr(pos+1, fn.size()-pos));
	else	return (fn);
}

// Normalization ops.
/* In header.
   template<typename T> T norm(T min, T cur, T max);
   template<typename T> T renorm(T min_in, T max_in, T cur, T min_out, T max_out);
   */

// Tokenization

std::vector<std::string> tokenize(const std::string & input_string,
                                  const std::string & delimiters, const char comment_marker,
                                  bool include_delimiters) {

	std::string str = input_string;

	// If str.size() == 0 return str

	if (str.find(comment_marker) != (uint32_t)-1)
		str.resize(str.find(comment_marker));

	size_t lastPos = str.find_first_not_of(delimiters, 0);
	size_t pos = str.find_first_of(delimiters, lastPos); // returns -1 if none

	if (pos == std::string::npos)
		return (std::vector<std::string>(1, str));

	std::vector<std::string> tokens;

	while (std::string::npos != pos || std::string::npos != lastPos) {
		if (include_delimiters && pos != str.size())
			tokens.push_back(str.substr(lastPos, pos + 1 -
			                            lastPos));
		else
			tokens.push_back(str.substr(lastPos, pos - lastPos));

		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}

	return (tokens);
}

std::string strip_spaces(const std::string & in) {

	size_t counter, rcount;

	// Find the first non-space, from the beginning
	for (counter = 0; counter < in.size() && in[counter] == ' '; ++counter);
	// Find the first non-space, from the end
	for (rcount = in.size()-1; rcount >= counter && in[rcount] == ' ';
	        --rcount);

	return (in.substr(counter, rcount-counter + 1));
}

// File slurping.
std::vector<std::string> slurp_file(std::ifstream & source,
                                    bool print_while_slurping) {
	std::vector<std::string> output;
	std::string next;

	while (!source.eof()) {
		getline(source, next);
		if (!source.eof()) {
			if (print_while_slurping)
				std::cout << next << std::endl;
			output.push_back(next);
		}
	}

	return (output);
}