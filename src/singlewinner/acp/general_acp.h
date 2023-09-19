// A generalized version of ACP: this takes an election method
// (method A), determines the winners, and for each winner,
// creates a Condorcet matrix truncated below this winner, and
// determines the outcome according to a pairwise method (method B),
// breaking ties by method A.

// Every candidate who won one of these elections is a winner in the
// final ordering. (I might also try just passing the results through
// Minmax.)

// ACP itself has A = Plurality (or Contingent vote) and
// B = Condorcet set.

// I'm for the most part interested in how well the adjusted
// Condorcet matrices play with more decisive Condorcet methods or
// with sets like Smith. This will destroy its LNH performance, I
// think(???), but it could still be interesting.

#include "../method.h"
#include "../sets/condorcet.h"
#include "../positional/simple_methods.h"

#include <memory>

class generalized_acp : public election_method {

	private:
		std::shared_ptr<pairwise_method> pairwise_base;
		std::shared_ptr<election_method> base_method;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		virtual std::string name() const {
			return "Generalized ACP: [" + base_method->name() + "], [" +
				pairwise_base->name() + "]";
		}

		generalized_acp() {
			base_method = std::make_shared<plurality>(
					plurality(PT_FRACTIONAL));
			pairwise_base = std::make_shared<condorcet_set>();
		}

		// Used for the TTR variant where (I imagine?) the top-two
		// winner is used as the threshold candidate. And since I
		// can make it generic, why not? Try IRV-ACP for instance...
		generalized_acp(
			std::shared_ptr<election_method> base_method_in,
			std::shared_ptr<pairwise_method> pw_base_in) {

			base_method = base_method_in;
			pairwise_base = pw_base_in;
		}

		// This is rather ugly and not at all how you're supposed
		// to create shared pointers, but keep it for now while the
		// rest of quadelect uses bare pointers.
		generalized_acp(
			election_method * base_method_in,
			pairwise_method * pw_base_in) {

			base_method = std::shared_ptr<election_method>(
					base_method_in);
			pairwise_base = std::shared_ptr<pairwise_method>(
					pw_base_in);
		}
};