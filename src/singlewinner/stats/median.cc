// Median ratings, the median version of Cardinal Ratings (range). We also
// suport the "gradual relaxation" tiebreak specified in
// http://wiki.electorama.com/wiki/Median_Ratings.

// Maybe we should move normalization elsewhere. Hm.

// UNTESTED

#include "median.h"

// Distance is the distance from the median that is to be included. If 0, only
// the median is included. If 0.1, we include from 0.4 times numvoters to 0.6.

double median_ratings::get_trunc_mean_destructively(
	std::vector<std::pair<double, double> > & array,
	double num_voters, bool already_sorted, double distance) const {

	// This has side effects, namely that we sort. Our strategy, because
	// we can't use nth element with weighted ballots, is to sort the array
	// and then go down until our count is >= num_voters/2. If it's exactly
	// num_voters/2, the weighted mean of this and the next is the median,
	// otherwise we're at the median.

	if (!already_sorted) {
		sort(array.begin(), array.end());
	}

	double wt_count = 0;

	std::vector<std::pair<double, double> >::const_iterator lower =
		array.end(),
		upper = array.end();

	for (std::vector<std::pair<double, double> >::const_iterator pos =
			array.begin();
		pos != array.end() && lower == array.end(); ++pos) {
		//std::cout << pos->first << "\t" << wt_count << " of " << num_voters << " ( linear: count " << pos - array.begin() << " of " << array.end() - array.begin() << ")" << std::endl;
		wt_count += pos->second;

		if (wt_count >= num_voters * 0.5) {
			if (wt_count == num_voters * 0.5) {
				lower = pos;
				upper = pos+1;
			} else {
				lower = pos;
				upper = pos;
			}
		}
	}

	std::cout << "DEBUG: " << lower - array.begin() << " " << upper -
		array.begin()
		<< " vs " << std::endl;
	std::cout << floor(num_voters * 0.5) << " " << ceil(num_voters * 0.5) <<
		std::endl;

	assert(lower != array.end());  // shouldn't happen.

	// First calculate the median (or average of bimedians).

	double median = (lower->first * lower->second +
			upper->first * upper->second) / (lower->second + upper->second);

	// Add mean of values just above and below (TEST).
	median += ((lower-1)->first + (upper+1)->first) * 0.00005;
	//median += ((lower-2)->first + (upper+2)->first) * 0.0005;

	return (median);
}

// Like in Cardinal Ratings, this determines each candidate's rating. Ratings
// for candidates not defined as hopeful will be zero.
std::vector<double> median_ratings::aggregate_ratings(
	const election_t &
	papers, int num_candidates,
	const std::vector<bool> & hopefuls) const {

	// Because the median isn't summable, we have to store a v<v<double> >
	// where one dimension is the candidates and the other the voters. We
	// then gather the ratings (normalized or not), then we find the median
	// of each, and then we're done.

	// Since we may have weighted ballots, we need a pair so that the
	// median calc can find the weighted median instead of the unweighted
	// one. The first double of the pair is the value, the second is the
	// weight.

	std::vector<std::vector<std::pair<double, double> > > ratings_block(
		num_candidates);

	ordering::const_iterator sec;

	// We must make this a vector since the voters may not necessarily be
	// expressing an opinion about every candidate.
	std::vector<double> num_voters(num_candidates, 0);
	double score;

	double global_max = -INFINITY, global_min = INFINITY;

	for (election_t::const_iterator pos = papers.begin(); pos !=
		papers.end(); ++pos) {

		// Do we need to normalize? If so, find the max and min hopeful
		// candidate so we have our range.
		double local_max = -INFINITY, local_min = INFINITY;

		if (normalize) {
			for (sec = pos->contents.begin(); sec != pos->contents.
				end(); ++sec) {
				if (!hopefuls[sec->get_candidate_num()]) {
					continue;
				}

				if (sec->get_score() > local_max) {
					local_max = sec->get_score();
				}

				if (sec->get_score() < local_min) {
					local_min = sec->get_score();
				}
			}
		}

		for (sec = pos->contents.begin(); sec != pos->contents.end();
			++sec) {
			if (!hopefuls[sec->get_candidate_num()]) {
				continue;
			}

			num_voters[sec->get_candidate_num()] += pos->get_weight();

			// If local_min == local_max, then the voter equal-ranks
			// everything, in which case normalizing would cause
			// a division by zero. Just take the equal-rank at face
			// value if that's the case.
			if (normalize && local_min != local_max)
				score = round(renorm(local_min, local_max,
							sec->get_score(),
							(double)range_minimum,
							(double)range_maximum));
			else	{
				score = sec->get_score();
			}

			score = std::min((double)range_maximum, std::max((double)
						range_minimum, score));

			global_min = std::min(global_min, score);
			global_max = std::max(global_max, score);

			ratings_block[sec->get_candidate_num()].push_back(
				std::pair<double, double>(score,
					pos->get_weight()));
		}

	}

	// Now determine the median ratings of all those candidates, and return
	// those.

	std::vector<double> retval(num_candidates, 0);

	for (int counter = 0; counter < num_candidates; ++counter)
		if (hopefuls[counter]) {
			if (ratings_block[counter].empty()) {
				retval[counter] = global_min;
			} else
				retval[counter] = get_trunc_mean_destructively(
						ratings_block[counter],
						num_voters[counter], false, 0);
		}

	return (retval);
}

std::pair<ordering, bool> median_ratings::elect_inner(
	const election_t & papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * cache, bool winner_only) const {

	// Get everybody's score. We don't know who's going to be the winner
	// ahead of time (*might* be possible through sorted structs but bleh),
	// so we have to get the answers for all the candidates.

	std::vector<double> scores = aggregate_ratings(papers, num_candidates,
			hopefuls);

	// Now we just have to turn it into an ordering.

	std::pair<ordering, bool> medians(ordering(), false);

	for (int counter = 0; counter < scores.size(); ++counter)
		if (hopefuls[counter])
			medians.first.insert(candscore(counter,
					scores[counter]));

	return (medians);
}

std::string median_ratings::determine_name() const {
	std::string base = "Median-" + itos(range_maximum - range_minimum) + "(";
	if (normalize) {
		base += "norm";
		if (tiebreak) {
			base += ", ";
		}
	}
	if (tiebreak) {
		base += "tiebreak)";
	} else	{
		base += ")";
	}

	return (base);
}

median_ratings::median_ratings(int min_in, int max_in, bool norm_in,
	bool tiebreak_in) {
	assert(min_in < max_in);

	range_minimum = min_in;
	range_maximum = max_in;
	normalize = norm_in;
	tiebreak = tiebreak_in;

	cached_name = determine_name(); // Must be run after any change!
}