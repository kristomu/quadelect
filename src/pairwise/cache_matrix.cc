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

		double get_internal(int candidate, int against, bool raw) const;
		bool set_internal(int candidate, int against, double value);

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

double cache_condmat::get_internal(int candidate, int against, bool raw) const {
	if (!is_loaded()) return(0);

	if (raw)
		return(reference->get_magnitude(candidate, against));
	else	return(type.transform(reference->get_magnitude(candidate, 
					against),
				reference->get_magnitude(against, candidate),
				reference->get_num_voters()));
}

// This won't work since the reference is a const. Therefore, we just give up
// early so the problem will be easily visible.
bool cache_condmat::set_internal(int /*candidate*/, int /*against*/, double /*value*/) {
	assert (1 != 1);
}

bool cache_condmat::set_parameters(const abstract_condmat * reference_in,
		pairwise_type in) {
	type = in;
	if (reference_in == NULL || reference_in->get_type().get() != 
			CM_PAIRWISE_OPP)
		return(false);

	reference = reference_in;
	return(true);
}

// Returns whether the cache_condmat is linked to anything.
bool cache_condmat::is_loaded() const {
	return (reference != NULL);
}

double cache_condmat::get_num_candidates() const {
	return(reference->get_num_candidates());
}

// We can't simply cache this locally, since the underlying matrix may change 
// at any time without us knowing.
double cache_condmat::get_num_voters() const {
	return(reference->get_num_voters());
}

cache_condmat::cache_condmat(const condmat * reference_in,
		pairwise_type kind) : abstract_condmat(kind) {

	assert (reference_in != NULL && 
			reference_in->get_type().get() == CM_PAIRWISE_OPP);
	reference = reference_in;
}

cache_condmat::cache_condmat(pairwise_type kind) : abstract_condmat(kind) {
	reference = NULL; // Nothing there yet.
}

#endif
