// Warren's "Vickrey auction" idea.
// Can be generalized to any sort of reweighting. The method is always house-
// monotone.

// [KM 2024: I'm not sure if this is implemented correctly. It's *very*
// majoritarian. But let's at least get it added.]

#include <assert.h>
#include "methods.h"
#include "tools/tools.h"

class r_auction : public multiwinner_method {
	private:
		int elect_and_update(std::vector<bool> & elected,
			std::vector<double> & account_balance,
			const election_t & ballots,
			double minimum, double maximum,
			int num_candidates, bool cumulative) const;

		bool cumul_setting;

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		r_auction(bool cumul_in) {
			cumul_setting = cumul_in;
		}

		std::string name() const {
			std::string ef;
			if (cumul_setting) {
				ef = "?? Cumul";
			} else {
				ef = "?? Ordinary";
			}
			return ("Range-Auction(" + ef + ")");
		}
};