// "Dirichlet model": Ranks according to scores v_a + v_b + ... = 1 with all
// v scores >= 0.
// Again, unsure if this is random for truncated ballots.

#ifndef _BALLOT_GENERATOR_DIR
#define _BALLOT_GENERATOR_DIR

#include "ballotgen.h"
#include <ext/numeric>

using namespace std;

class dirichlet : public indiv_ballot_generator {
	private:
		ordering generate_ordering(int numcands,
				bool do_truncate, rng & random_source) const;

	public:
		dirichlet() : indiv_ballot_generator() {}
		dirichlet(bool compress_in) : 
			indiv_ballot_generator(compress_in) {}
		dirichlet(bool compress_in, bool trunc) : 
			indiv_ballot_generator(compress_in, trunc) {}

		string name() const { return("Dirichlet"); }
};
#endif
