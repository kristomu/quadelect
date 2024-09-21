#include "prbucklin.h"

std::list<size_t> set_pr_bucklin::count(
	const std::vector<std::vector<int> > & ballots,
	const std::vector<double> & weights, size_t numcands, size_t seats) const {

	std::vector<double> score_so_far(numcands, 0);
	std::vector<double> adjust(numcands, 0);
	std::vector<bool> selected(numcands, false);
	size_t num_selected = 0;

	std::list<size_t> council;

	size_t sum_weights = 0, counter, sec;

	for (counter = 0; counter < weights.size(); ++counter) {
		sum_weights += weights[counter];
	}

	double quota = sum_weights / (double)(seats+1);

	for (size_t cur_round = 0; cur_round < numcands
		&& num_selected < seats; ++cur_round) {

		for (sec = 0; sec < ballots.size(); ++sec) {
			if (ballots[sec].size() <= cur_round) {
				continue;
			}

			score_so_far[ballots[sec][cur_round]] += weights[sec];
		}

		std::vector<std::pair<double, int> > rank(numcands);

		for (sec = 0; sec < numcands; ++sec)
			if (!selected[sec]) {
				// Pushing
				rank.push_back(std::pair<double, int>(
						score_so_far[sec],
						sec));
			} else {
				// Opt variant: push score_so_far[sec] - adjust[sec].
				rank.push_back(std::pair<double, int>(
						score_so_far[sec] - adjust[sec], sec));
			}

		bool electable = true;

		while (num_selected < seats && electable) {

			sort(rank.rbegin(), rank.rend()); // tiebreak?

			electable = (rank[cur_round].first >=
					quota);// && !selected[rank[cur_round].second]);

			if (!electable) {
				continue;
			}

			// pick greatest unelected candidate
			bool done = false;
			size_t necount = 0;
			for (sec = 0; necount <= cur_round && !done && sec < rank.size(); ++sec) {

				assert(rank[sec].first >= 0);
				if (rank[sec].first < quota) {
					continue;
				}

				if (selected[rank[sec].second]) {
					continue;
				}

				council.push_back(rank[sec].second);
				selected[rank[sec].second] = true;
				rank[sec].first -= quota;
				adjust[rank[sec].second] += quota;
				done = true;
				++num_selected;
				++necount;
			}
			electable = necount > 0;
		}
	}

	assert(council.size() == seats);

	return (council);
}

std::list<size_t> set_pr_bucklin::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & ballots) const {

	// Doesn't handle equal rank, etc.

	std::vector<double> weights;
	std::vector<std::vector<int> > support;

	for (election_t::const_iterator pos = ballots.begin(); pos !=
		ballots.end(); ++pos) {
		weights.push_back(pos->get_weight());
		std::vector<int> this_support;
		for (ordering::const_iterator spos = pos->contents.begin();
			spos != pos->contents.end(); ++spos) {
			this_support.push_back(spos->get_candidate_num());
		}
		support.push_back(this_support);
	}

	return (count(support, weights, num_candidates, council_size));
}
