// Extended cache for methods testing. Currently we cache:
// 	- outcomes
// 	- pairwise matrices.
// TODO: Generalize so we can store all sorts of stuff, e.g. positional
// matrices, Range arrays, Plur counts with eliminated candidates, you name it.
//
// The cache helps cut down on time when we want to find out the same result
// many times, such as with determining the Smith set for numerous Smith,X
// methods.

#ifndef _VOTE_CACHE
#define _VOTE_CACHE

#include <unordered_map>
#include <vector>
#include <map>

#include "tools/ballot_tools.h"
#include "pairwise/matrix.h"
#include "pairwise/cache_matrix.h"

// This is for the outcome. First is full, second is winner only.
typedef pair<ordering, ordering> cache_orderings;

class cache_map {

	private:
		unordered_map<string, cache_orderings> outcomes;

		// This is a list so we can detect it if it's empty. We'll
		// do something more proper later, possibly with links to names
		// for beatpath etc.

		list<condmat> condorcet_cache;

	public:
		// The set/get functions are inline because they get called
		// *a lot*.

		inline void set_outcome(const string & name, bool winner_only,
			const ordering & outcome);
		inline void set_outcome(const string & name,
			const pair<ordering, bool> & outcome_inf);

		bool has_outcome(const string & name) const;
		bool has_outcome(const string & name, bool winner_only) const;

		inline pair<ordering, bool> get_outcome(const string & name,
			bool winner_only) const;

		// Condorcet cache

		bool set_condorcet_matrix(const condmat & input);
		bool has_condorcet_matrix() const;
		cache_condmat get_condorcet_cache(pairwise_type kind) const;

		void clear();
};

// Inline functions go here because otherwise the compiler can't find them in
// time.

inline void cache_map::set_outcome(const string & name, bool winner_only,
	const ordering & outcome) {

	if (winner_only) {
		outcomes[name].second = outcome;
	} else {
		outcomes[name].first = outcome;
	}
}

inline void cache_map::set_outcome(const string & name,
	const pair<ordering,
	bool> & outcome_inf) {

	set_outcome(name, outcome_inf.second, outcome_inf.first);
}

// If there's no cache, it'll return empty. Otherwise:
//      - If winner_only is false, it either returns a full ranking or nothing.
//      - If winner_only is true, it returns a full ranking if it exists,
//              otherwise a winner-only, otherwise nothing. (We might want
//              to make it work the opposite way to uncover bugs with
//              winner_only, but well.. not yet.)
inline pair<ordering, bool> cache_map::get_outcome(const string & name,
	bool winner_only) const {

	unordered_map<string, cache_orderings>::const_iterator lookup =
		outcomes.find(name);

	if (lookup == outcomes.end()) {
		return (pair<ordering, bool>(ordering(), false));
	}

	if (!lookup->second.first.empty()) {
		return (pair<ordering, bool>(lookup->second.first, false));
	}

	if (winner_only && !lookup->second.second.empty()) {
		return (pair<ordering, bool>(lookup->second.second, true));
	}

	return (pair<ordering, bool>(lookup->second.first, false));
}

#endif
