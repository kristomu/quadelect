
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <string>
#include <math.h>
#include <list>
#include <set>
#include <map>

#include "ballots.h"

// Ballot components. An ordering is just that, but it also includes ratings
// (if so declared), so that loser-elimination/average-elimination can work,
// and so that we may experiment with cardinal ratings later.

// We enforce that scores are finite because this makes manipulating their
// values easier, e.g. when doing tiebreaks.

void candscore::set_score(double score_in) {
	if (isnan(score_in)) {
		throw std::invalid_argument("candscore: NaN is not a valid score.");
	}
	if (!isfinite(score_in)) {
		throw std::invalid_argument("candscore: Scores must be finite.");
	}
	score = score_in;
}

void candscore::set_candidate_num(size_t candnum_in) {
	candidate_number = candnum_in;
}

candscore::candscore(size_t candnum_in) {
	score = 0;
	set_candidate_num(candnum_in);
}

candscore::candscore(size_t cn_in, double score_in) {
	set_candidate_num(cn_in);
	set_score(score_in);
}

///////////////////////////////////////////////////////////////////////////////
// Ballot group.

ballot_group::ballot_group() {
	weight = 1;
	complete = false;
	rated = false;
}

ballot_group::ballot_group(double weight_in) {
	set_weight(weight_in);
	complete = false;
	rated = false;
}

ballot_group::ballot_group(double weight_in, const ordering & cont,
	bool complete_in, bool rated_in) {
	set_weight(weight_in);
	contents = cont;
	complete = complete_in;
	rated = rated_in;
}
