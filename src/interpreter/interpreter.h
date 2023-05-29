#ifndef __VOTE_INTERPRET
#define __VOTE_INTERPRET

// Abstract base class for interpreters. Interpreters are classes that take
// text input of some form and returns a set of ballots (and possibly candidate
// names). They're used as part of determining winners according to given
// ballots and election methods.

// TODO: Somehow support limited interpreters, e.g. CONDMAT_ROW, CONDMAT_COL,
// positional matrices...

#include "../ballots.h"
#include "../tools/tools.h"

#include <vector>
#include <list>
#include <map>

using namespace std;

class interpreter {

	public:
		// Ternary with value for "don't know"?
		virtual bool is_this_format(const vector<string> &
			inputs) const = 0;

		virtual pair<map<size_t, string>, list<ballot_group> >
		interpret_ballots(const vector<string> & inputs,
			bool debug) const = 0;

		virtual string name() const = 0;

		virtual ~interpreter() {}

		// virtual v<s> explain ?
};

#endif
