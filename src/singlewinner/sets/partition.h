// Partition set. This set is used to perform the "preround" of Conitzer &
// Sandholm 03, which is supposed to make a method more strategy resistant
// (at the cost of a bunch of criteria). There are two modes: in the first,
// the preround violates symmetry of candidates but always works the same
// way (used for strategy tests). In the second, the seed order is random.

// The preround consists of paritioning the candidate set into two. Each
// candidate is paired off against his opposite number and the one who beats the
// other candidate pairwise, wins. Leftover candidates are given a bye.

#ifndef _SET_PARTITION
#define _SET_PARTITION

#include <vector>

#include "../method.h"
#include "../pairwise/method.h"

using namespace std;

class partition_set : public pairwise_method {
	private:
		bool is_random;

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls, 
				cache_map * cache, bool winner_only) const;

		void set_random(bool be_random);

		partition_set(bool be_random) : pairwise_method(CM_WV) { 
			set_random(be_random); }

		string pw_name() const;
};

#endif
