#include "../method.h"
#include "../../pairwise/matrix.h"
#include "method.h"

using namespace std;

string pairwise_method::type_suffix() const {
	return("(" + default_type.explain() + ")");
}

string pairwise_method::determine_name() const {
	if (type_matters)
		return(pw_name() + type_suffix());
	else    return(pw_name());
}

void pairwise_method::update_name() {
	if (do_cache_name)
		cached_name = determine_name();
}

pair<ordering, bool> pairwise_method::pair_elect(const abstract_condmat & input,
		cache_map & cache, bool winner_only) const {

	return(pair_elect(input, vector<bool>(input.get_num_candidates(), 
					true), cache, winner_only));
}


// DONE: Cache the matrices and also make a "raw matrix" form that can be turned
// into whatever type you want (CM_MARGINS, CM_WV, etc). Then look it up and use
// the cached matrix if available.
// Should speed things up by a lot, in particular if we can then use add/
// subtract ops to modify the matrix according to a modifications list when 
// doing two-tests (e.g. mono-raise: instead of constructing two different 
// Condorcet matrices, subtract the original ballot from it and add the one 
// where some candidate was raised).

// Eugh, cut and paste code... maybe I should group these together with a v<b>
// even though I'll then pay the price by having to allocate them all the time.

pair<ordering, bool> pairwise_method::elect_inner(
		const list<ballot_group> & papers, int num_candidates,
		cache_map & cache, bool winner_only) const {

	// If the cache is non-NULL and it has a Condorcet matrix, make use of
	// it. (TODO: Names here)

	if (&cache != NULL) {
		if (cache.has_condorcet_matrix())
			return(pair_elect(cache.get_condorcet_cache(
							default_type), 
						cache, winner_only));

		// If we have a cache but it hasn't stored anything, make a
		// Condorcet matrix - Pairwise opposition, because that's the
		// only thing the cache will store - and dump it into the
		// cache so we can use it later.
		
		condmat archetype(papers, num_candidates, CM_PAIRWISE_OPP);
		cache.set_condorcet_matrix(archetype);
		return(pair_elect(archetype, cache, winner_only));
	}

	// If there's no cache, generate the Condorcet matrix and pass it to
	// the pairwise election routine.

	return(pair_elect(condmat(papers, num_candidates, default_type), 
				cache, winner_only));
}

pair<ordering, bool> pairwise_method::elect_inner(
		const list<ballot_group> & papers,
		const vector<bool> & hopefuls, int num_candidates,
		cache_map & cache, bool winner_only) const {

	// DONE: Fix in matrix.cc so we can have things like Small. Very
	// wasteful unless the matrix just looks up the hopeful list and
	// nulls out on-the-fly, but we'll do smart things later.
	// Now we do that.

	// See above for caching.
	if (&cache != NULL) {
		if (cache.has_condorcet_matrix())
			return(pair_elect(cache.get_condorcet_cache(
							default_type),
						hopefuls, cache, winner_only));

		 condmat archetype(papers, num_candidates, CM_PAIRWISE_OPP);
		 cache.set_condorcet_matrix(archetype);
		 return(pair_elect(archetype, hopefuls, cache, winner_only));
	}

	return(pair_elect(condmat(papers, num_candidates, default_type), 
				hopefuls, cache, winner_only));
}

// DONE: Find out how to move this to "determine_name" without running into
// inheritance trouble.
// We just use pw_name to give a name and then handle everything behind the 
// scenes. The only thing the coder has to do is to call update_name() after any
// parameter change.

string pairwise_method::name() const {
	if (do_cache_name)
		return(cached_name);
	else	return(determine_name());
}
