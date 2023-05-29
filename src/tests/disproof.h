// Disproof structure for two-tests. A disproof contains all the required data
// to show that a particular election method does not pass a particular
// two-test criterion. See the two-test code for explanation of what a two-test
// is.

#ifndef _TWOTEST_DISP
#define _TWOTEST_DISP

//#include "../multiwinner/methods.cc"
#include "../tools/ballot_tools.h"
#include "../singlewinner/method.h"
#include <vector>

using namespace std;

struct disproof {
	list<ballot_group> unmodified_ballots, modified_ballots;
	ordering unmodified_ordering, modified_ordering;
	vector<int> modification_data;
	bool complete;
};

class method_test_info {
	public:
		bool passes_so_far;
		int iters_run;
		disproof crit_disproof;

		method_test_info() {
			passes_so_far = true;
			iters_run = 0;
			crit_disproof.complete = false;
		}
};

#endif
