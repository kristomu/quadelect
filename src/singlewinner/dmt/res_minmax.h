#pragma once

// Resistant Minmax, an experimental resistant set method with low
// monotonicity failure rate.

// Let a path with margin p be defined as in the indirect disqualification
// set:

// Let there be a /path/, A -> X_1 -> ... -> X_n if
//	A disqualifies X_1 on every subelection that has both in them, with margin
//		at least p;
//		(i.e. fpA > 1/|S| + p on all such elections)
//
//	and then for all X_k, for X_(k-1) -> X_k to hold the following must be true:
//		For all subelections S:
//			If X_k is not in S, no constraint.
//			Let C be the earliest candidate on the path that's in the
//				subelection, and Q be the set of C and all subsequent candidates
//				on the path.
//			If every candidate in Q is in S, then the sum of the first preferences
//				of candidates in Q must be greater than |Q|/|S| + p.

// A candidate A's score vector is, for each other candidate X, the greatest
// margin p (which may be negative) for which there is a path from A to X with
// margin p.

// The candidate with the greatest leximax set of scores wins.

// The implementation below is very hacky, so beware.

#include "singlewinner/sets/inner_burial.h"
#include "singlewinner/pairwise/method.h"
#include "singlewinner/pairwise/simple_methods.h"
#include "singlewinner/pairwise/kemeny.h"
#include <memory>

typedef enum { RM_LEXIMAX = 0, RM_STRENGTH = 1, RM_LSTRENGTH = 2, RM_STRLEX = 3, RM_CONDORCET = 4 } rm_type;

class res_minmax : public election_method {
	private:
		// For determining what candidates indirectly
		// disqualify others.

		std::shared_ptr<pairwise_method> base_method;
		rm_type type;

		bool tiebreak = false;

		void explore_paths(
			const std::vector<bool> & hopefuls,
			const subelections & se,
			std::vector<size_t> & chain_members,
			std::vector<bool> & chain_members_bool,
			size_t leaf,
			std::vector<bool> & disqualified) const;

		std::vector<std::vector<bool> > explore_all_paths(
			const election_t & papers,
			const std::vector<bool> & hopefuls) const;

		// Quick and dirty

		void set_path_strengths(
			const std::vector<bool> & hopefuls,
			const subelections & se,
			std::vector<size_t> & chain_members,
			std::vector<bool> & chain_members_bool,
			size_t leaf, double disq_strength,
			std::vector<double> & root_strengths) const;

		std::vector<std::vector<double> > set_all_path_strengths(
			const election_t & papers,
			const std::vector<bool> & hopefuls) const;

		// Helper functions.

		bool has_cycle(const std::vector<std::vector<bool> > &
			disqualifications, std::vector<bool> & visited,
			size_t cur_cand) const;

		bool has_cycle(const std::vector<std::vector<bool> > &
			disqualifications) const;

		void print(const std::vector<std::vector<bool> > &
			disqualifications) const;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			std::string pfx;

			if (!tiebreak) {
				pfx = "Tie-";
			}

			switch (type) {
				case RM_LEXIMAX: return pfx+"Resistant-Leximax";
				case RM_STRENGTH: return pfx+"Resistant-Strength";
				case RM_LSTRENGTH: return pfx+"Resistant-Leximax-Str";
				case RM_STRLEX: return "Resistant-Str-Leximax";
				default: return pfx+"Resistant-CM(" + base_method->name() + ")";
			}
		}

		res_minmax(std::shared_ptr<pairwise_method> base_method_in) {
			base_method = base_method_in;
			type = RM_CONDORCET;
		}

		res_minmax(rm_type type_in) {
			base_method = std::make_shared<kemeny>(CM_PAIRWISE_OPP);
			type = type_in;
		}

		res_minmax() {
			type = RM_STRENGTH;
		}
};
