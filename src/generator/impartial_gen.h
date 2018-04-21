// Extensible ballot generator for impartial culture. The generator returns
// every ordering with probability sampled from a given distribution function.
// This thus neatly generalizes both impartial culture (IC) and Dirichlet
// (sampling from an exponential function).
// If truncation is off, everything works as above. If it's on, it also uses a
// kludge to truncate in some cases and not in others. It should really extend
// the concept to draw every ballot (truncated or not) with equal probability
// (before weight biasing), but doing so is hard.

// BLUESKY: Do that with both truncation and equal-rank. That's even harder!

#ifndef _VOTE_BG_IMPARTIAL_GEN
#define _VOTE_BG_IMPARTIAL_GEN

#include "ballotgen.h"
#include <list>

using namespace std;

class impartial_gen : public pure_ballot_generator {

	private:
		list<ballot_group> generate_ballots_int(
		    int num_voters, int numcands,
		    bool do_truncate, rng & random_source) const;

	protected:
		virtual double get_sample(rng & random_source) const = 0;

	public:
		impartial_gen(bool compress_in) :
			pure_ballot_generator(compress_in) {}
		impartial_gen(bool compress_in, bool do_truncate) :
			pure_ballot_generator(compress_in, do_truncate) {}
};
#endif
