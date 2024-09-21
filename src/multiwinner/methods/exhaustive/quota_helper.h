// This is a helper (not really a method) intended to pick every council
// that passes Droop proportionality criteria. It's basically a copy
// of set_webster_helper; trying to make a base class to specialize into
// bnoth this and set_webster_helper probably wouldn't be useful with only
// two examples.

#include "coalitions/coalitions.h"
#include "method.h"

class quota_helper : public exhaustive_method {
	private:
		bool maximize() const {
			return true;
		}

		double evaluate(combo::it & start, combo::it & end);

		std::vector<coalition_data> solid_coalitions;
		size_t council_size;

	public:
		void process_ballots(const election_t & ballots,
			size_t num_candidates) {

			solid_coalitions = get_solid_coalitions(
					ballots, std::vector<bool>(num_candidates, true),
					num_candidates);
		}

		quota_helper(size_t council_size_in) {
			council_size = council_size_in;
		}

		// Required due to the way the exhaustive method runner works.
		// Don't use this; it will throw an error.
		quota_helper() {
			council_size = 0;
		}

		std::string name() const {
			return "Droop Quota Helper";
		}
};

double quota_helper::evaluate(combo::it & start, combo::it & end) {
	std::set<size_t> council(start, end);

	// Assumes that we can get the total weight from the first solid
	// coalition, i.e. that they're in sorted order. TODO, fix so that
	// if I change this later, it will trigger instead of silently fail.
	// Or get the total weight (num voters) from somewhere else.

	double total_weight = solid_coalitions.begin()->support;

	if (council_size == 0) {
		throw std::runtime_error("Quota helper: council size not set!");
	}

	for (auto cc_pos = solid_coalitions.begin();
		cc_pos != solid_coalitions.end(); ++cc_pos) {

		// See PSC-CLE.
		size_t minimum = ceil(cc_pos->support *
				(council_size + 1) / total_weight - 1);

		size_t elect_constraint = std::min(
				cc_pos->coalition.size(),
				minimum);

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

typedef exhaustive_method_runner<quota_helper> quota_determiner;
