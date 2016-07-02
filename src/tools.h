#ifndef __TOOLS
#define __TOOLS

#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <assert.h>
#include <sstream>
#include <algorithm>
#include <vector>

using namespace std;

const double PI = 4 * atan(1);

// Templated functions must be included in the header so the compiler knows what
// to expand.

template<typename T> T square(const T a) { return(a*a); }

template<typename T> T inc(T in) {
	        return(++in);
}

double get_abs_time();
// Distance on the Lp norm (generalized Euclidean).
double euc_distance(double Lp, const vector<double> & a, 
		const vector<double> & b);

// Integer to string

string itos (int source);
string lltos(long long source);
string dtos (double source);
string dtos (double source, double precision);
string s_padded(string a, int maxlen);
string s_right_padded(string a, int maxlen);
string itos(int source, unsigned int minlen);
string lltos(long long source, unsigned int minlen);

// Hexadecimal versions of the above

string itos_hex(int source);
string lltos_hex(long long source);
string itos_hex(int source, unsigned int minlen);
string lltos_hex(long long source, unsigned int minlen);

// String to integer.

// For comparison purposes
// (Perhaps include a warning that the number will wrap around?)
// Perhaps also alias both to a common function, to promote code reuse.
// Two levels: strict only returns is_integer on shorts. Normal returns them
// on anything. 

template<class T> void arch_stoi(T & dest, bool hex, string source) {
        stringstream q;
        if (hex)
                q.flags(ios::hex);
        q << source;
        q >> dest;
        return;
}

long long comp_stoi(string source);
unsigned int str_toui(string source);
int str_toi(string source);
double str_tod(string source);
long long comp_stoi_hex(string source);
int str_toi_hex(string source);
int str_toi_generalized(string source);
bool is_integer(string source, bool permit_hex);

// Misc string modifications
string lowercase(string mixed);
string uppercase(string mixed);
string remove_extension(string fn);
string remove_path(string fn);

// Normalization ops.
template<typename T> T norm(T min, T cur, T max) {
        if (max == min) return(0);
        return((cur-min)/(max-min));
}

template<typename T> T renorm(T min_in, T max_in, T cur, T min_out, T max_out) {
        return(norm(min_in, cur, max_in) * (max_out-min_out) + min_out);
}

// Tokenization

vector<string> tokenize(const string & input_string, const string &
		delimiters, const char comment_marker,
		bool include_delimiters);

string strip_spaces(const string & in);

#endif
