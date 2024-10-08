#include "common/ballots.h"
#include "../method.h"
#include <list>
#include <vector>

#include "tools/ballot_tools.h"
#include "aggregator.h"

void positional_aggregator::aggregate(const ballot_group & input,
	int num_candidates, int num_hopefuls, const std::vector<bool> &
	hopefuls, std::vector<std::vector<double> > & positional_array,
	positional_type kind, int zero_run_advice) const {

	int cur_idx = 0;

	ordering::const_iterator pos = first_hopeful(
			input.contents.begin(), input.contents.end(),
			hopefuls);

	ordering::const_iterator end_group = pos;

	// Keep on going until the end or until we know all the
	// remaining ranks will only gather zeroes. The "or" here
	// ignores the zero_run_beginning()'s advice if there is none.
	while (pos != input.contents.end() && (cur_idx < zero_run_advice
			|| zero_run_advice == -1)) {
		// First determine how many equal-rank voters we have,
		// if any, for this rank.
		int span = 0;

		while (end_group != input.contents.end() &&
			end_group->get_score() == pos->get_score()) {
			end_group = next_hopeful(end_group,
					input.contents.end(), hopefuls);
			++span;
		}

		// Now end_group is the first that's not equally ranked.
		// Add the equal-ranked ones to the tally. This'll be
		// one if none were equally ranked.
		for (; pos != end_group; pos = next_hopeful(pos,
					end_group, hopefuls)) {

			double value = input.get_weight();
			if (kind == PT_FRACTIONAL) {
				value /= (double)span;
			}

			positional_array[pos->get_candidate_num()][cur_idx] +=
				value;
		}

		cur_idx += span;
	}
}

std::vector<std::vector<double> >
positional_aggregator::get_positional_matrix(
	const election_t & ballots, int num_candidates,
	int num_hopefuls, const std::vector<bool> & hopefuls,
	positional_type kind, int zero_run_advice) const {

	// First dimension the matrix. It's num_candidates * minimum ofnumber
	// of places (num_candidates) and the zero_run_advice.
	// For instance, a plurality matrix is x1 because we only count the
	// first place.
	// Note that we can't dimension it to num_hopefuls, since the hopefuls
	// may be candidates 1 and 10, for instance, and so dimensioning it
	// to 2 (num_hopefuls) would cause an error upon writing to 10.

	int width = zero_run_advice;
	if (width == -1) {
		width = num_candidates;
	}

	std::vector<std::vector<double> > positional_matrix(num_candidates,
		std::vector<double>(width, 0));

	// For all the ballots, incorporate into the matrix.

	for (election_t::const_iterator bpos = ballots.begin();
		bpos != ballots.end(); ++bpos)
		aggregate(*bpos, num_candidates, num_hopefuls, hopefuls,
			positional_matrix, kind, zero_run_advice);

	return (positional_matrix);
}

std::vector<std::vector<double> >
positional_aggregator::get_positional_matrix(const
	election_t & ballots, int num_candidates,
	int num_hopefuls, const std::vector<bool> & hopefuls,
	positional_type kind) const {

	return (get_positional_matrix(ballots, num_candidates, num_hopefuls,
				hopefuls, kind, -1));
}
