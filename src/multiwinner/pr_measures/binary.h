#include "measure.h"
#include <vector>

// This proportionality measure only works on binary spatial models, but
// is pretty simple. It just checks the proportion of the voters that
// agree with each issue (take the true or 1 position) and compares this
// to the proportion of the winners that do.

// TODO? Maybe a print function???

class binary_proportionality : public proportionality_measure {
	private:
		// These vectors ought to be bool but because all spatial
		// generators work on double, we use what we're given.

		// Each candidate's opinion (true = 1, false = 0) on
		// every issue.
		std::vector<std::vector<double> > candidate_opinions;

		// The fraction of voters and winners who take the true
		// position on each issue.
		std::vector<double> issue_voter_proportions;
		std::vector<double> issue_winner_proportions;

	public:
		void prepare(const positions_election & p_e);
		double get_error(const std::list<size_t> & outcome);
};