#include "cache_matrix.h"
#include <stdexcept>

// This is a "cache Condorcet matrix". It appears like any other Condorcet
// matrix, using any kind of pairwise type, but internally links to a cached
// pairwise opposition type matrix.

// NOTE: This code may be very old and/or not in use. Find out later which
// is the case.

double cache_condmat::get_internal(size_t candidate, size_t against,
	bool raw) const {
	if (!is_loaded()) {
		return 0;
	}

	if (raw) {
		return reference->get_magnitude(candidate, against);
	} else {
		return (type.transform(reference->get_magnitude(candidate,
						against),
					reference->get_magnitude(against, candidate),
					reference->get_num_voters()));
	}
}

// This won't work since the reference is a const. Therefore, we just give up
// early so the problem will be easily visible.
bool cache_condmat::set_internal(size_t /*candidate*/, size_t /*against*/,
	double /*value*/) {
	throw std::domain_error("cache_condmat: can't directly manipulate"
		" read-only cache references!");
}

bool cache_condmat::set_parameters(const abstract_condmat * reference_in,
	pairwise_type in) {
	type = in;
	if (reference_in == NULL || reference_in->get_type().get() !=
		CM_PAIRWISE_OPP) {
		return false;
	}

	reference = reference_in;
	return true;
}

// Returns whether the cache_condmat is linked to anything.
bool cache_condmat::is_loaded() const {
	return reference != NULL;
}

size_t cache_condmat::get_num_candidates() const {
	return reference->get_num_candidates();
}

// We can't simply cache this locally, since the underlying matrix may change
// at any time without us knowing.
double cache_condmat::get_num_voters() const {
	return reference->get_num_voters();
}

cache_condmat::cache_condmat(const condmat * reference_in,
	pairwise_type kind) : abstract_condmat(kind) {

	// Caching a matrix that isn't pairwise opposition shouldn't
	// happen. Doing so is a coding error, and so should be asserted.

	if (reference_in == NULL) {
		throw std::invalid_argument("cache_condmat: Can't create a matrix "
			" with a null reference!");
	}

	if (reference_in->get_type().get() != CM_PAIRWISE_OPP) {
		throw std::invalid_argument("cache_condmat: Reference matrix "
			" must be pairwise opposition type!");
	}
	reference = reference_in;
}

cache_condmat::cache_condmat(pairwise_type kind) : abstract_condmat(kind) {
	reference = NULL; // Nothing there yet.
}