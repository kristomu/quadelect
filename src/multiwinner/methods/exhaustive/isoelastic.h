#pragma once

#include "scored_method.h"
#include "tools/tools.h"

class isoelastic_eval : public scored_method {
	private:
		void add_method_score(const combo::it & start,
			const combo::it & end,
			const scored_ballot & this_ballot,
			score_aggregate & output);

		double r;

	public:
		std::string name() const {
			return "Cardinal: Isoelastic (r = " + dtos(r) + ")";
		}

		isoelastic_eval() {
			r = 1;
		}

		isoelastic_eval(double r_in) {
			r = r_in;
		}
};

inline void isoelastic_eval::add_method_score(const combo::it & start,
	const combo::it & end, const scored_ballot & this_ballot,
	score_aggregate & output) {

	/* Peter Zbornik's suggestion of

	   F=( s(1)^(r-1)+...+s(n)^(r-1) ) / (r-1),

	   where s(k) is the number of elected candidates that the kth
	   voter approved of.

	   We can rewrite this as sum over voters: s(v)^(r-1) / (r-1).

	   If r = 1, then this per-voter term is just log(s(v)),
	   taking the limit.

	   From
	   http://lists.electorama.com/pipermail/election-methods-electorama.com/2010-May/124448.html
	*/

	// For Range, we currently just generalize ratings as fractional
	// approvals. This ideally needs a max and min value, because ratings
	// outside the interval would distort the function. Play with that
	// later.

	double total_rating = 0;

	for (auto pos = start; pos != end; ++pos) {
		total_rating += this_ballot.get_norm_score(*pos);
	}

	if (r == 1) {
		if (total_rating == 0) {
			output.insert(this_ballot.weight,
				-std::numeric_limits<double>::infinity());
			return;
		}

		output.insert(this_ballot.weight,
			log(total_rating));
		return;
	}

	output.insert(this_ballot.weight, pow(total_rating, r-1)/(r-1));
}

//typedef exhaustive_method_runner<isoelastic_eval> isoelastic;
