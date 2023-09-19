// Impartial anonymous culture: This is the discrete version of Dirichlet:
// Let x_1, ..., x_n be the different ballots. Then uniformly generate a point on the
// lattice of the n-simplex defined by x_1 + ... + x_n = |V| and x_k >= 0 for all k,
// where |V| is the number of voters.

// Note that this doesn't support truncation or equal rank at the moment.
// It should be pretty easy to generalize if I could construct a bijection between
// ballots with equal rank and/or truncation and integers. But no dice so far.

#pragma once

#include "ballotgen.h"
#include "impartial_gen.h"
#include <list>
#include <stdexcept>

class iac : public pure_ballot_generator {
	protected:
		election_t generate_ballots_int(int num_voters,
			int numcands, bool do_truncate, coordinate_gen & coord_source) const;

	public:
		std::string name() const {
			return ("Impartial Anonymous Culture");
		}

		iac(bool compress_in) :
			pure_ballot_generator(compress_in) {}
		iac(bool compress_in, bool do_truncate) :
			pure_ballot_generator(compress_in, do_truncate) {}
};