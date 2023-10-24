#pragma once

#include "../method.h"
#include "../../ballots.h"
#include "../../tools/tools.h"

#include <list>
#include <memory>


// This method is a generalization of Forest Simmons' Duncan method
// to handle different base methods and deal with ties in it in a
// reasonable manner.

// Forest's definition of this class of methods is:
// Take some base method (for Duncan it's Borda). Based on the
// base method's social ordering, elect the highest ranked candidate
// who pairwise beats everybody ranked below him.

// My generalization is to elect the candidate who pairwise beats or
// ties everybody up to and including his own rank. Everybody else
// is tied for last.

// I'll try to find a way to construct an actual ordering later.
// Suppose W is our winner. The main problem is that we don't know
// where to rank the candidates ranked above W by the base method.

// For instance, if the base method ranking is A>B>C>D and
// pairwise we have C>D, B>C, B>D, A>C, A>D, then it seems
// clear that our output social order should be B>C>D. But where
// does A go? All we really know is that it can't be above B.

// A possible generalization could be this:
//		Everybody who beats or ties everybody below him ranks in the
//		top tier. Ties are broken by how many candidates there are.
//		Then everybody who beats or ties everybody but one.
//		Then everybody who beats or ties everybody but two, etc.

// But that would make the social order B>C>D>A, which seems absurd.

// Alternatively:
//		Everybody who beats everybody below first rank.

// argh, do this later.

class beat_chain : public election_method {
	private:
		std::shared_ptr<const election_method> base;

	protected:
		std::pair<ordering, bool> elect_inner(const
			election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

	public:

		beat_chain(std::shared_ptr<const election_method> base_method) {
			base = base_method;
		}

		std::string name() const {
			return "EXP: Beat Chain-[" + base->name() + "]";
		}
};