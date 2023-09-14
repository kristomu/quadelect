// Dirichlet instantiation of the generalized class: we return ballots with
// weights drawn from the standard exponential distribution (by inverting the
// CDF).

// Currently much too slow. I have to find out whether that's due to the
// construction or that Dirichlet ballots simply render most methods
// resistant to strategy, i.e. that it's the strategy detection that takes
// time.

#ifndef _VOTE_BG_DIRICHLET
#define _VOTE_BG_DIRICHLET

#include "ballotgen.h"
#include "impartial_gen.h"
#include <list>


class dirichlet : public impartial_gen {

	protected:
		double get_sample(rng & random_source) const {
			return (-log(1 - random_source.next_double()));
		}

	public:
		std::string name() const {
			return ("Dirichlet");
		}

		dirichlet(bool compress_in) :
			impartial_gen(compress_in) {}
		dirichlet(bool compress_in, bool do_truncate) :
			impartial_gen(compress_in, do_truncate) {}

};
#endif
