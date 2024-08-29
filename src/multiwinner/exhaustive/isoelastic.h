#include "scored_method.h"
#include "tools/tools.h"

class isoelastic_eval : public scored_method {
	private:
		double evaluate(combo::it & start, combo::it & end,
			const scored_ballot & this_ballot);

		double r;

	public:
		std::string name() const {
			return "Cardinal: Isoelastic (r = " + dtos(r) + ")";
		}

		bool maximize() const {
			return true;
		}

		isoelastic_eval() {
			r = 1;
		}

		isoelastic_eval(double r_in) {
			r = r_in;
		}
};

double isoelastic_eval::evaluate(combo::it & start, combo::it & end,
	const scored_ballot & this_ballot) {

	/* Peter Zbornik's suggestion of

	   F=( s(1)^(r-1)+...+s(n)^(r-1) ) / (r-1),

	   where s(k) is the number of elected candidates that the kth
	   candidate approved of.

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
		// We should return an infinite value, but doing it like this
		// lets us tie-break on the finite values.
		if (total_rating == 0) {
			return this_ballot.weight * -1e15;
		}

		return this_ballot.weight * log(total_rating);
	} else {
		return this_ballot.weight * pow(total_rating, r-1)/(r-1);
	}
}

typedef exhaustive_method_runner<isoelastic_eval> isoelastic;