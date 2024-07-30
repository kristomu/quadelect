#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <map>
#include <set>

#include <assert.h>

// This should be located somewhere else, because it isn't really
// related to linear model constraints. (Or should it?)
// Anyway, going to do that later.

// This class represents a mapping of input candidates to output candidates.
// It's used for stating what candidates are analogous before and after
// a transformation by a relative criterion.

// For instance, an independence of clones test might take candidates
// (A, B, C) and turn them into (A, B, C, D) so that D is a clone of B.
// Then D is analogous to B, so testing if A is vulnerable to teaming
// could consist of checking if A is ranked above B before cloning and
// after D after the cloning (in which case, the method is vulnerable to
// teaming).

// For independence criteria, there's an additional snag. For instance,
// proving mono-raise through ISDA might take the form of "If we start
// with a three-cycle, introduce a candidate D who's not in the Smith set,
// and raise A so D is included in the Smith set, then D must not beat A".
// But the candidate D is invisible to the ISDA test while D is not a part
// of the Smith set, so we need to somehow encode that a candidate who's
// not yet scored by the method appears later, so that we can test for
// such violations of ISDA (e.g. in minimax). That's what the eliminated
// candidate token CP_NONEXISTENT does. If the pair (CP_NONEXISTENT, D)
// exists, then that means that a candidate that doesn't exist in the
// beginning is introduced into the system as D in the after-ballot.


///////////////////////////////////////////////////////////

typedef ptrdiff_t ssize_t;	/* ssize_t is not part of the C standard */

const ssize_t CP_NONEXISTENT = -1;

class cand_pairs {
	private:
		// If pairs[x] contains y, then that means that candidate index x
		// before modification corresponds to candidate index y after. A
		// given candidate before may correspond to many after (e.g. when
		// cloning) or none (when eliminating that candidate).

		// Since we sometimes need wildcard queries and sometimes need to
		// iterate over all pairs, the data is stored in two structures.
		// Since they're private, the outside won't have to deal with the
		// complexity.
		std::map<ssize_t, std::set<ssize_t> > pairs;
		std::multimap<ssize_t, ssize_t> all_pairs;

		size_t num_source = 0;

	public:

		void set_pair(ssize_t before, ssize_t after) {
			// Don't count the elimination pseudo-candidate as a real
			// source candidate.
			if (pairs.find(before) == pairs.end() &&
				before != CP_NONEXISTENT) {
				++num_source;
			}
			pairs[before].insert(after);
			all_pairs.insert(std::make_pair(before, after));
		}

		size_t num_source_candidates() const {
			return num_source;
		}

		std::multimap<ssize_t, ssize_t>::const_iterator begin() const {
			return all_pairs.begin();
		}

		std::multimap<ssize_t, ssize_t>::const_iterator end() const {
			return all_pairs.end();
		}

		std::set<ssize_t>::const_iterator begin_by_cand(ssize_t cand) const {
			assert(pairs.find(cand) != pairs.end());
			return pairs.find(cand)->second.begin();
		}

		std::set<ssize_t>::const_iterator end_by_cand(ssize_t cand) const {
			assert(pairs.find(cand) != pairs.end());
			return pairs.find(cand)->second.end();
		}

		size_t num_after_cands_by_before(ssize_t cand) const {
			if (pairs.find(cand) == pairs.end()) {
				return 0;
			}
			return pairs.find(cand)->second.size();
		}

		void print_pairs() const {
			for (const auto & x: pairs) {
				std::cout << x.first << " -> ";

				std::copy(x.second.begin(), x.second.end(),
					std::ostream_iterator<ssize_t>(std::cout, " "));

				std::cout << std::endl;
			}
		}
};