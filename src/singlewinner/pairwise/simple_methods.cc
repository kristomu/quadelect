
#include "../method.h"
#include "pairwise/matrix.h"

#include "simple_methods.h"

#include <complex>

//////////////////////////////

// Some simple (low K-complexity) Condorcet methods follow.

// Minmax
// Pick the candidate for which the greatest opponent magnitude is the least.
// This produces a social ordering according to the greatest opponent magnitude,
// negated (so as to keep with the convention that higher score is better).

std::pair<ordering, bool> ord_minmax::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	ordering toRet;

	bool debug = false;

	// Being able to generalize argmin/argmax here would have been very
	// nice.
	// Could try winner_only here, but for now, just return the full result.
	for (int counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}

		int record_pos = 0;

		for (int sec = 1; sec < input.get_num_candidates(); ++sec) {
			if (input.get_magnitude(sec, counter, hopefuls) >
				input.get_magnitude(record_pos,
					counter, hopefuls)) {
				record_pos = sec;
			}
		}

		if (debug) {
			std::cout << "ord_minmax: candidate " << counter
				<< " has maximal defeat " <<
				input.get_magnitude(record_pos, counter) << " by " <<
				record_pos << std::endl;
		}

		if (reverse_perspective) {
			toRet.insert(candscore(counter, input.get_magnitude(
						counter, record_pos)));
		} else {
			toRet.insert(candscore(counter, -input.get_magnitude(
						record_pos, counter)));
		}
	}

	return (std::pair<ordering, bool>(toRet, false));
}

// Ext-minmax and minmin.

std::pair<ordering, bool> ext_minmax::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	// For all candidates, generate a vector of scores against them.
	// Sort this vector so that greatest come first, then sort the pairs
	// to sort by closest defeat.

	// Again, we could speed up winner_only, but we'll do so later and see
	// if there's any benefit.

	bool debug = false;

	std::vector<std::pair<std::vector<double>, int> > scores;

	size_t counter;

	std::pair<std::vector<double>, int> beat_data;
	beat_data.first.resize(input.get_num_candidates());

	for (counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}

		for (size_t sec = 0; sec < input.get_num_candidates(); ++sec)
			if (counter != sec && hopefuls[sec])
				beat_data.first[sec] = (input.
						get_magnitude(sec, counter,
							hopefuls));
			else {
				beat_data.first[sec] = 0;
			}

		if (debug) {
			std::cout << "Before sorting: " << counter << "\t";
			copy(beat_data.first.begin(), beat_data.first.end(),
				std::ostream_iterator<double>(std::cout, "   "));
			std::cout << std::endl;
		}

		// Sort this descending if Minmax, ascending if Minmin
		if (minmin)
			sort(beat_data.first.begin(), beat_data.first.end(),
				std::less<double>());
		else
			sort(beat_data.first.begin(), beat_data.first.end(),
				std::greater<double>());

		beat_data.second = counter;
		scores.push_back(beat_data);
	}

	// Sort by least greatest score (if Minmax) or least least score
	// (if Minmin).
	sort(scores.begin(), scores.end());

	if (debug) {
		for (size_t counter = 0; counter < scores.size(); ++counter) {
			std::cout << "After sorting: " << scores[counter].second
				<< "\t";
			copy(scores[counter].first.begin(),
				scores[counter].first.end(),
				std::ostream_iterator<double>(std::cout, "   "));
			std::cout << std::endl;
		}
	}

	ordering to_ret;

	// Start adding to the output ordering
	int rank = 0;
	for (counter = 0; counter < scores.size(); ++counter) {
		// If it's not first rank and the previous score array is
		// different from this one, then we're worse than the previous
		// one, otherwise it's equal rank.
		if (counter != 0)
			if (scores[counter].first != scores[counter-1].first) {
				--rank;
			}

		to_ret.insert(candscore(scores[counter].second, rank));
	}

	return (std::pair<ordering, bool>(to_ret, false));
}

// "Maxmin"
std::pair<ordering, bool> maxmin::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	ordering toRet;

	for (int counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}
		int record_pos = -1, first = -1;

		for (int sec = 0; sec < input.get_num_candidates(); ++sec) {
			if (!hopefuls[sec]) {
				continue;
			}
			if (counter == sec) {
				continue;
			}
			if (first == -1) {
				first = sec;
			}

			if ((record_pos == -1 || input.get_magnitude(counter,
						sec, hopefuls) <
					input.get_magnitude(counter,
						record_pos,
						hopefuls)) &&
				input.get_magnitude(counter, sec,
					hopefuls) != 0) {
				record_pos = sec;
			}
		}

		if (record_pos == -1) {
			record_pos = first;    // force 0 if loser
		}

		toRet.insert(candscore(counter, input.get_magnitude(counter,
					record_pos)));

	}

	return (std::pair<ordering, bool>(toRet, false));
}

// Copeland and n-th order Copeland. WV, Margins, PO doesn't matter.
// Also may be extended to sports versions (3-1 and 2-1).
std::string copeland::pw_name() const {
	std::string base;
	if (order == 1) {
		base = "Copeland";
	} else	{
		base = itos(order) + ". order Copeland";
	}

	if (win == 1 && tie == 0) {
		return (base);
	} else	{
		return (base + "/" + dtos(win) + "/" + dtos(tie));
	}
}

std::vector<double> copeland::get_copeland(const abstract_condmat & input,
	const std::vector<bool> & hopefuls,
	const std::vector<double> & counterscores) const {

	std::vector<double> scores(input.get_num_candidates(), 0);

	for (int counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}
		int wins = 0, ties = 0;
		for (int sec = 0; sec < input.get_num_candidates(); ++sec) {
			if (!hopefuls[sec]) {
				continue;
			}
			if (input.get_magnitude(counter, sec, hopefuls) >
				input.get_magnitude(sec, counter,
					hopefuls)) {
				wins += counterscores[sec];
			} else if (input.get_magnitude(counter, sec, hopefuls) ==
				input.get_magnitude(sec, counter,
					hopefuls)) {
				ties += counterscores[sec];
			}
		}

		scores[counter] = wins * win + ties * tie;
	}

	return (scores);
}

std::pair<ordering, bool> copeland::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	ordering toRet;

	// To make ordinary Copeland for the first round.
	std::vector<double> scores(input.get_num_candidates(), 1);

	for (unsigned int cur_order = 0; cur_order < order; ++cur_order) {
		scores = get_copeland(input, hopefuls, scores);
	}

	// Spool it all into the ordering
	for (int counter = 0; counter < input.get_num_candidates(); ++counter)
		if (hopefuls[counter]) {
			toRet.insert(candscore(counter, scores[counter]));
		}

	return (std::pair<ordering, bool>(toRet, false));
}

// Schulze!

std::pair<ordering, bool> schulze::pair_elect(const abstract_condmat &
	input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	beatpath bpath(input, CM_PAIRWISE_OPP, hopefuls);

	// Count defeats.
	int i, j, numcand = bpath.get_num_candidates();
	std::vector<int> defeats(numcand, 0);

	for (i = 0; i < numcand; ++i)
		for (j = 0; j < numcand; ++j)
			if (i != j && hopefuls[i] && hopefuls[j] &&
				bpath.get_magnitude(j, i, hopefuls) >
				bpath.get_magnitude(i, j, hopefuls)) {
				++defeats[i];
			}

	// The standard is "greater score is better", so we need to invert,
	// since more defeats are worse, not better.

	ordering social_ordering;

	for (i = 0; i < numcand; ++i)
		if (hopefuls[i]) {
			social_ordering.insert(candscore(i, -defeats[i]));
		}

	return (std::pair<ordering, bool>(social_ordering, false));
}
