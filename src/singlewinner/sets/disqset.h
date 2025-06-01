#pragma once

// The indirect disqualification set, an experimental set that's currently
// a more monotone resistant set, but with the aim of eventually
// becoming a fully monotone set or method.

// Let there be a /path/, A ~> X_1 ~> ... ~> X_n if
//	A disqualifies X_1 on every subelection that has both in them;
//
//	and then for all X_k:
//		X_(k-1) disqualifies X_k in every subelection containing
//			X_(k-1) and X_k
//		The combined first preferences of the k candidates
//			A ... X_(k-1) exceeds k/|S| in every subelection
//			containing all of them plus X_k.

// Then the indirect disqualification set is everybody who isn't at the
// receiving end of a path.

// It's monotone in the sense that if A has a path to X_n, then raising
// A can't break that path. It should also be acyclical for four candidates,
// nothing guaranteed for greater numbers. The code will test if there's a
// top cycle and throw an exception if so.

// Currently it has a transitivity failure which in turn produces
// monotonicity problems when paired with arbitrary methods. Suppose A has
// a path to C, and B has no paths to (or from) anybody. Then the set is
// {A, B}. Suppose we raise B on an A>B>C ballot. Then A's path to C may
// disappear, making the set {A, B, C}, hence causing a monotonicity
// failure if something like (Indirect disqualification),Minmax is used
// and Minmax's order is B>A>C.

// EXPERIMENTAL. The set's definition may change at any time!

#include "inner_burial.h"

class idisqualif_set : public election_method {
	private:
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
			return "Indirect disqualification set";
		}
};