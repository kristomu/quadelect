#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>

#include <assert.h>

#include "methods.h"

class set_pr_bucklin : public multiwinner_method {
	private:
		std::list<size_t> count(const std::vector<std::vector<int> > & ballots,
			const std::vector<double> & weights, size_t numcands,
			size_t seats) const;

	public:
		std::list<size_t> get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("Set PR Bucklin");
		}
};