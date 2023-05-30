#ifndef _VOTE_SW_COMMA
#define _VOTE_SW_COMMA

// Comma - meta-method with which to build methods like Smith,IRV.
// The meta-method first calculates the outcome of the first method (which is
// usually a set, like Smith), and then breaks the "ties" - tiers of set
// membership - according to the second method.

#include "../method.h"
#include "../../ballots.h"
#include "../../tools/ballot_tools.h"

class comma : public election_method {

	private:
		const election_method * set_method;
		const election_method * specific_method;

		std::string cached_name;

	protected:
		std::pair<ordering, bool> elect_inner(const std::list<ballot_group> &
			papers, const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string determine_name() const;

	public:

		// A bit unintuitively, the constructor has the specific method
		// first and the set second, but eh, backwards compatibility!
		comma(const election_method * specific_method_in,
			const election_method * set_in) {
			set_method = set_in;
			specific_method = specific_method_in;
			cached_name = determine_name();
		}

		std::string name() const {
			return (cached_name);
		}
};

#endif
