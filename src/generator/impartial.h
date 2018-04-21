// Impartial culture instantiation of the generalized class: we return ballots
// with weight 1 for all, so that each permutation is equally likely. We could
// also use the uniform distribution, but it would just use more time.

#ifndef _VOTE_BG_IMPARTIAL
#define _VOTE_BG_IMPARTIAL

#include "ballotgen.h"
#include "impartial_gen.h"
#include <list>

using namespace std;

class impartial : public impartial_gen {

	protected:
		double get_sample(rng & random_source) const {
			return (1);
		}

	public:
		string name() const {
			return ("Impartial Culture");
		}

		impartial(bool compress_in) :
			impartial_gen(compress_in) {}
		impartial(bool compress_in, bool do_truncate) :
			impartial_gen(compress_in, do_truncate) {}

};
#endif
