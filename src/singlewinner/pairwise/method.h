#ifndef _VOTE_P_METHOD
#define _VOTE_P_METHOD

#include "../method.h"
#include "../../pairwise/matrix.h"

#include <complex>

using namespace std;

// DONE: Make a generic name() function that asks for a specific pair_name()
// function and adds stuff like Margins afterwards unless overridden by a bool
// that says type doesn't matter (like in the case of Copeland). This would have
// the interesting side effect that if you do happen to declare Copeland twice
// with different types, the cache will just look up the earlier result for the
// latter.

// To note: Some Condorcet methods may not be able to elect from the Condorcet
// matrix alone. One example is BTR-IRV. Thus not all Condorcet methods can
// run pair_elect - handle this ambiguity later. DONE by renaming this
// "pairwise_method" from "condorcet_method".

// The method only performs a cache lookup when called from elect_inner, so
// that if you manually pass it a certain Condorcet matrix, it will work upon
// that matrix instead of just assuming it's equal to the cached matrix.
// However, some methods may cache or use derived matrices (which is why we pass
// cache into the core function), and they will do that regardless of whether
// you're passing a Condorcet matrix or a list of ballots. Beware that and
// don't pass the cache if you don't want that behavior.

// Remember to call update_name() once you're done initializing everything in
// your constructors.

class pairwise_method : public election_method {
	protected:
		pairwise_type default_type;
		string cached_name;

		bool type_matters; // TODO, make a criterion thing instead.
		bool do_cache_name;

		string type_suffix() const;

		// Handle cached names. This is required because cache ops
		// request names for every hit/miss check, there are a lot of
		// them, and merging strings is *slow*.
		string determine_name() const;
		void update_name();

	public:
		// This one's extended by the method in question.
		// DONE: Support winner_only by passing it on.
		// TODO: Refuse if the input matrix is of the wrong type. (??)
		virtual pair<ordering, bool> pair_elect(
			const abstract_condmat & input,
			const vector<bool> & hopefuls,
			cache_map * cache,
			bool winner_only) const = 0;

		pair<ordering, bool> pair_elect(const abstract_condmat & input,
			const vector<bool> & hopefuls,
			bool winner_only) const {
			return (pair_elect(input, hopefuls, NULL, winner_only));
		}

		pair<ordering, bool> pair_elect(const abstract_condmat & input,
			cache_map * cache,
			bool winner_only) const;

		pair<ordering, bool> pair_elect(const abstract_condmat & input,
			bool winner_only) const {
			return (pair_elect(input, NULL, winner_only));
		}

		pair<ordering, bool> elect_inner(
			const list<ballot_group> & papers,
			int num_candidates, cache_map * cache,
			bool winner_only) const;
		pair<ordering, bool> elect_inner(
			const list<ballot_group> & papers,
			const vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		pairwise_type get_type() const {
			return (default_type);
		}

		pairwise_method(pairwise_type def_type_in) {
			default_type = def_type_in; type_matters = true;
			do_cache_name = true; cached_name = "/ERROR/NOT-SET/";
		}

		// One often gathers a bunch of pairwise_methods
		// (or election_methods, even) in an array of pointers to the
		// ABC. This ensures the inherited classes' destructors are
		// called if one performs delete ops on that array.
		virtual ~pairwise_method() {}

		virtual string pw_name() const = 0;

		string name() const;
};

#endif
