// Ballot generator for impartial culture. This returns every ordering with
// equal probability if truncation is off. If it's on, it also uses a kludge
// to truncate in some cases and not in others. It should really extend the
// concept to draw every ballot (truncated or not) with equal probability, but
// doing so is hard.

// BLUESKY: Do that with both truncation and equal-rank. That's even harder!

#ifndef _VOTE_BG_IMPARTIAL
#define _VOTE_BG_IMPARTIAL

#include "ballotgen.h"
#include <list>

using namespace std;

class impartial : public pure_ballot_generator {

	private:
		list<ballot_group> generate_ballots_int(
				int num_voters, int numcands,
				bool do_truncate, rng & random_source) const;

	public:
		impartial(bool compress_in) : 
			pure_ballot_generator(compress_in) {}
		impartial(bool compress_in, bool do_truncate) : 
			pure_ballot_generator(compress_in, do_truncate) {}

		string name() const { return("Impartial Culture"); }
};
#endif
