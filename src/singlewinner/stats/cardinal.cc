#include "cardinal.h"

// Cardinal Ratings (Range).

// -----

// This determines each candidate's rating. Ratings for candidates that are not
// defined as hopeful will be zero. [TODO: Something about this when all scores
// are negative.]
std::vector<double> cardinal_ratings::aggregate_ratings(
	const election_t &	papers, int num_candidates,
	const std::vector<bool> & hopefuls) const {

	std::vector<double> ratings(num_candidates, 0);

	ordering::const_iterator sec;

	double score;

	for (election_t::const_iterator pos = papers.begin(); pos !=
		papers.end(); ++pos) {
		// Assert that we're dealing with a rated ballot. Maybe using
		// a throw would be better, but I don't know those.
		//assert(pos->rated);

		// Do we need to normalize? If so, find the max and min
		// hopeful so we can renorm.

		// (Perhaps renormalize on a voter-wide level if norm is off,
		//  so that if everybody votes on a range -e to +pi/2, it gets
		//  properly converted. Do that later?)
		double local_max = 0, local_min = 0;
		bool set_max = false, set_min = false;

		if (normalize) {
			for (sec = pos->contents.begin(); sec != pos->contents.
				end(); ++sec) {
				if (!hopefuls[sec->get_candidate_num()]) {
					continue;
				}

				if (sec->get_score() > local_max || !set_max) {
					local_max = sec->get_score();
					set_max = true;
				}

				if (sec->get_score() < local_min || !set_min) {
					local_min = sec->get_score();
					set_min = true;
				}
			}

		}

		for (sec = pos->contents.begin(); sec != pos->contents.end();
			++sec) {
			if (!hopefuls[sec->get_candidate_num()]) {
				continue;
			}

			// If local_min == local_max, then that means the voter
			// equal-ranks everything, and so we should take his
			// rating at face value. Therefore, we don't actually
			// normalize unless there's a range to normalize.
			if (normalize && local_min != local_max)
				score = round(renorm(local_min, local_max,
							sec->get_score(),
							(double)minimum,
							(double)maximum));
			else	{
				score = sec->get_score();
			}

			score = std::min((double)maximum, std::max((double)minimum,
						score));

			ratings[sec->get_candidate_num()] += pos->get_weight() *
				score;
		}
	}

	return (ratings);
}

std::pair<ordering, bool> cardinal_ratings::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// No matter whether it's winner-only or not, return the full ranking
	// (as we take only a very minor hit for doing so).

	std::vector<double> rating_totals = aggregate_ratings(papers,
			num_candidates,
			hopefuls);

	// Now all we have to do is turn that into an ordering.
	std::pair<ordering, bool> toRet;
	toRet.second = false; // whole rank, not winner

	for (size_t counter = 0; counter < rating_totals.size(); ++counter)
		if (hopefuls[counter])
			toRet.first.insert(candscore(counter,
					rating_totals[counter]));

	return (toRet);
}

std::string cardinal_ratings::name() const {
	std::string base = "Cardinal-" + dtos(maximum-minimum);
	if (normalize) {
		base += "(norm)";
	}

	return base;
}

cardinal_ratings::cardinal_ratings(int min_in, int max_in, bool norm_in) {
	assert(min_in < max_in);

	minimum = min_in;
	maximum = max_in;
	normalize = norm_in;
}
