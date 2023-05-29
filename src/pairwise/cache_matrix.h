#ifndef _VOTE_C_CMATRIX
#define _VOTE_C_CMATRIX

#include "matrix.h"

// This is a "cache Condorcet matrix". It appears like any other Condorcet
// matrix, using any kind of pairwise type, but internally links to a cached
// pairwise opposition type matrix.

// If you give this a matrix that's not of the pairwise opposition, it'll either
// bail with an assertion failure, or return false.

class cache_condmat : public abstract_condmat {

	protected:
		const abstract_condmat * reference;

		double get_internal(size_t candidate, size_t against, bool raw) const;
		bool set_internal(size_t candidate, size_t against, double value);

	public:

		bool set_parameters(const abstract_condmat * reference_in,
			pairwise_type in);

		bool is_loaded() const;
		double get_num_candidates() const;
		double get_num_voters() const;

		cache_condmat(const condmat * reference_in,
			pairwise_type kind);
		cache_condmat(pairwise_type kind);
};

#endif
