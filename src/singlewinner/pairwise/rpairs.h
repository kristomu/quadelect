// Ranked Pairs / MAM. The difference between this method and "canonical" MAM
// is that it doesn't break wv ties by margins - simply because we would want
// to use it on other things than wv. It doesn't produce a random voter
// hierarchy either, yet.

// This thing is slow, somewhere between n^3 log n and n^4. The steps are:
// 	- Read all combinations and put them into an array.
//	- Sort by magnitude.
//	- Going from the top to the bottom, lock each pair unless it would
//		cause a cycle. Use a depth-first search to determine if it
//		would.
//	- Once done, start at the node with indegree zero (the winner), then
//		go down to get the ordering.

// (Now also contains River, even though it could be made quicker as a method
//  of its own since the River tree is undirected. Also, it doesn't augment as
//  in River+.)

// Ranked Pairs:

// TIDEMAN, T. Nicolaus. Independence of clones as a criterion for voting
// rules. Social Choice and Welfare, 1987, 4.3: 185-206.

// MAM:		http://alumnus.caltech.edu/~seppley/
// River:	http://lists.electorama.com/pipermail/
//				election-methods-electorama.com/2004-October/014018.html

// TODO: Return full ranking. I have to fix this before I make it available
// through the command line interface. I should also add a parameter that
// switches between the non-neutral tiebreak and the neutral but not cloneproof
// non-polytime version.

#pragma once

#include "pairwise/matrix.h"
#include "method.h"
#include "../method.h"

#include <iostream>
#include <list>

class beat_component {
	public:
		int winner, loser;
		double magnitude;

		bool operator< (const beat_component & other) const {
			return (magnitude < other.magnitude);
		}

		bool operator> (const beat_component & other) const {
			return (magnitude > other.magnitude);
		}

		beat_component(int winner_in, int loser_in, double magn) {
			winner = winner_in;
			loser = loser_in;
			magnitude = magn;
		}
};

class ranked_pairs : public pairwise_method {

	private:
		bool is_river;

		// Cache *might* make this quicker, but who knows?
		bool st_connectivity(int source, int dest,
			const std::vector<std::list<int> > &
			adjacency_lists) const;
		void traverse_tree(std::vector<int> & places, int node, int depth,
			const std::vector<std::list<int> > &
			adjacency_lists) const;

	public:
		ranked_pairs(pairwise_type def_type_in, bool river_in) :
			pairwise_method(def_type_in) {
			is_river = river_in;
			update_name();
		}
		std::string pw_name() const;

		std::pair<ordering, bool> pair_elect(const abstract_condmat & iput,
			const std::vector<bool> & hopefuls,
			cache_map * cache, bool winner_only) const;
};
