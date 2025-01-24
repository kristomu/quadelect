#pragma once

#include "common/ballots.h"
#include "tools/tools.h"

#include "types.h"
#include "abstract_matrix.h"

#include <iterator>
#include <iostream>


// TODO in some way or other: Cardinal weighted pairwise.
// Direction of defeat is determined as in Copeland. Say A > B. Then the magn.
// of (B>A) is 0 and the magn. of A>B is the sum of, for those who ranked A
// above B, the difference between A's score and B's score.
// Perhaps through a combination matrix. The combination matrix would check if
// a>b or a<b in primary. If equal, it checks in the secondary. Leximax classes
// become more and more appealing.

// Condorcet matrix class.

// I had to redo the entire class because it wasn't salvagable.
// This is an implementation of a basic Condorcet matrix. It contains functions
// to read and write values; however, these values are always concealed behind
// the type (wv, margins, etc) that the Condorcet matrix has been set to. The
// concealing is meant to keep election methods from prodding at the raw
// matrix, so that one can be sure that when one passes a wv modified matrix,
// that is what the method will use.

// It also contains count_ballots, which will reinitialize the matrix according
// to the ballots that are given to it. (Perhaps make that a function exposed
// in abstract_condmat, so that CPO-STV etc work similarly...)

// TODO: Consider the idea of using +/- to enable summing matrices (as in
// summability). Maybe also use that along with a virtual individual Condorcet
// matrix that derives info from an actual ballot to make ballot counting very
// easy, unless that would lead to a performance hit.
// But not all multiwinner "council matrix" systems are weakly summable.
// CPO-STV is not, for instance.

// TODO: Make set increase num_voters, or expose set_num_voters. We'll do the
// latter for now.

// TODO: Grant the matrix a name. This name should include the hopefuls array
// if it needs state to store the hopefuls (e.g. Beatpath only considering the
// hopefuls).

class condmat : public abstract_condmat {
	private:
		std::vector<std::vector<double> > contents;

	protected:
		double get_internal(size_t candidate, size_t against, bool raw) const;
		bool set_internal(size_t candidate, size_t against, double value);

	public:
		condmat(pairwise_type type_in);
		condmat(const election_t & scores, size_t num_candidates,
			pairwise_type kind);
		// Should we permit condmat(input, kind)? Does that break
		// or enhance encapsulation?
		// Do it for now, then judge later.
		condmat(const condmat & in, pairwise_type type_in);
		condmat(size_t num_candidates_in, double num_voters_in,
			pairwise_type type_in);

		// Eh, is this bad?
		bool add(size_t candidate, size_t against, double value) {
			return (set_internal(candidate, against,
						get_internal(candidate, against, true) +
						value));
		}

		void count_ballots(const election_t & scores,
			size_t num_candidates);

		// Perhaps "expand candidates by one, contract by one" here?
		// Clear, etc...

		double get_num_candidates() const {
			return (contents.size());
		}

		void zeroize();
		void zeroize(size_t num_candidates);

		void set_type(pairwise_type type_in) {
			type = type_in;
		}
};