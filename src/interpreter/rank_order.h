#ifndef __VOTE_INT_RO
#define __VOTE_INT_RO

// This is the interpreter class for ballots of the form
//	[number]: cand (> or =) cand ... cand
// or just        cand (> or =) cand ... cand
//
// For an example of the former, consider
//		  2: A > B = C
// which means two voters voted A first, then B equal second to C;
// and of the latter,
//		  C = B > A
// which means a single voter voted C and B equal-first and above A.

// Note that unless the first line of the format is RANK_ORDER, the candidates
// must all have alphanumeric names, and no special symbols apart from =:.> are
// permitted.

#include "interpreter.h"

using namespace std;

class rank_order_int : public interpreter {

	public:
		bool is_this_format(const vector<string> & inputs) const;

		pair<map<int, string>, list<ballot_group> > interpret_ballots(
		    const vector<string> & inputs,
		    bool debug) const;

		string name() const {
			return ("Raw rank-ballot input");
		}
};

#endif
