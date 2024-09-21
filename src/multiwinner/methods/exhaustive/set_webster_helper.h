// The Set Webster method works like this:
//	Find the largest value x so that, when solid coalitions' support values are
//	transformed by y = round(support * x), there exists at least one council
//	assignment of the desired size that passes the criteria
//		"For every solid coalition S, at least min(|S|, y(S)) members from this
//		 coalition must be elected."

// Since I don't know an algorithm to determine if such a council exists, I just
// try every possible set inside a bisection search that finds the greatest value
// of x. This helper makes this easy to do by making use of the exhaustive method
// infrastructure. It is not intended as a standalone method!

#include "coalitions/coalitions.h"
#include "method.h"

class set_webster_helper : public exhaustive_method {
	private:
		bool maximize() const {
			return true;
		}

		double evaluate(combo::it & start, combo::it & end);

		std::vector<coalition_data> solid_coalitions;
		double factor; // the x in the explanation above

	public:
		void process_ballots(const election_t & ballots,
			size_t num_candidates) {

			solid_coalitions = get_solid_coalitions(
					ballots, std::vector<bool>(num_candidates, true),
					num_candidates);
		}

		set_webster_helper() {
			factor = -1;
		}

		set_webster_helper(double factor_in) {
			factor = factor_in;
		}

		std::string name() const {
			return "Set Webster Helper";
		}
};

double set_webster_helper::evaluate(combo::it & start, combo::it & end) {
	// TODO: Optimization where we can signal that we're done as soon as
	// we find a satisfying coalition set. When doing the bisection search,
	// we're not interested in how many feasible solutions we can find,
	// only whether there is one or not. This will require some rearranging
	// of exhaustive_method, though.

	std::set<size_t> council(start, end);

	for (auto cc_pos = solid_coalitions.begin();
		cc_pos != solid_coalitions.end(); ++cc_pos) {

		size_t elect_constraint = std::min(cc_pos->coalition.size(),
				(size_t)round(factor * cc_pos->support));

		if (elect_constraint == 0) {
			continue;
		}

		std::set<size_t> proposed_in_coalition;
		std::set_intersection(
			council.begin(), council.end(),
			cc_pos->coalition.begin(), cc_pos->coalition.end(),
			std::inserter(proposed_in_coalition,
				proposed_in_coalition.begin()));

		if (proposed_in_coalition.size() < elect_constraint) {
			return -1000;
		}
	}

	return 1000;
}