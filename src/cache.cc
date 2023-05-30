#include <unordered_map>
#include <vector>
#include <map>

#include "tools/ballot_tools.h"
#include "pairwise/matrix.h"
#include "pairwise/cache_matrix.h"

#include "cache.h"

bool cache_map::has_outcome(const std::string & name) const {
	return (outcomes.find(name) != outcomes.end());
}

// Winner_only will also accept a full ordering, so if w_o is true, this
// reduces to the above function. Otherwise, we check that a full ranking is
// indeed available.
bool cache_map::has_outcome(const std::string & name,
	bool winner_only) const {
	bool at_all = has_outcome(name);

	if (winner_only) {
		return (at_all);
	}
	if (!at_all) {
		return (false);
	}

	return (!outcomes.find(name)->second.first.empty());
}

// Condorcet matrix caching.
bool cache_map::set_condorcet_matrix(const condmat & input) {
	// The Condorcet matrix must be set to be Pairwise Opposition so that
	// our "kinda-Condorcet matrix" can emulate any and all types.

	if (input.get_type().get() != CM_PAIRWISE_OPP) {
		return (false);
	}

	condorcet_cache.clear();
	condorcet_cache.push_back(input);
	return (true);
}

bool cache_map::has_condorcet_matrix() const {
	return (!condorcet_cache.empty());
}

cache_condmat cache_map::get_condorcet_cache(pairwise_type kind) const {
	// If we don't have anything cached, return a shell of a cache_matrix.
	// (Not really useful, so check first!)
	if (!has_condorcet_matrix()) {
		return (cache_condmat(kind));
	}
	// Otherwise, return a cache matrix referring to our matrix!
	return (cache_condmat(&(*condorcet_cache.begin()), kind));
}


void cache_map::clear() {
	outcomes.clear();
	condorcet_cache.clear();
}
