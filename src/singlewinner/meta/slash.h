#ifndef _VOTE_SW_SLASH
#define _VOTE_SW_SLASH

// Slash - meta-method with which to build methods like Smith/IRV.
// The meta-method first calculates the outcome of the first method (which is
// usually a set, like Smith), and then calculates the outcome of the second,
// constrained to the set of winners according to the first method. Then the
// outcome is the result of the second method, with ties broken according to
// the first.
// NOTE: There may be bugs in how hopefuls work for various methods. Fix later.
// BLUESKY: Iterated. E.g. say Smith returns A = B = C > D = E = F > G = H = I
//	then the result is the outcome for {A,B,C} with ties broken by the
//	outcome for {A,B,C,D,E,F} and so on.

#include "../method.h"
#include "../../ballots.h"
#include "../../ballot_tools.h"

class slash : public election_method {

	private:
		const election_method * set_method;
		const election_method * specific_method;

		string cached_name;

	protected:
		pair<ordering, bool> elect_inner(const list<ballot_group> &
				papers, const vector<bool> & hopefuls,
				int num_candidates, cache_map * cache,
				bool winner_only) const;

		string determine_name() const;

	public:

		// A bit unintuitively, the constructor has the specific method
		// first and the set second, but eh, backwards compatibility!
		slash(const election_method * specific_method_in,
				const election_method * set_in) {
			set_method = set_in; 
			specific_method = specific_method_in;
			cached_name = determine_name(); }

		string name() const { return(cached_name); }
};

#endif
