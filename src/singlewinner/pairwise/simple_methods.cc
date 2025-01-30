
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
	for (size_t candidate = 0; candidate < input.get_num_candidates();
		++candidate) {

		if (!hopefuls[candidate]) {
			continue;
		}

		size_t record_pos = 0;

		for (size_t challenger = 1; challenger < input.get_num_candidates();
			++challenger) {

			if (input.get_magnitude(challenger, candidate, hopefuls) >
				input.get_magnitude(record_pos,
					candidate, hopefuls)) {
				record_pos = challenger;
			}
		}

		if (debug) {
			std::cout << "ord_minmax: candidate " << candidate
				<< " has maximal defeat " <<
				input.get_magnitude(record_pos, candidate) << " by " <<
				record_pos << std::endl;
		}

		if (reverse_perspective) {
			toRet.insert(candscore(candidate, input.get_magnitude(
						candidate, record_pos)));
		} else {
			toRet.insert(candscore(candidate, -input.get_magnitude(
						record_pos, candidate)));
		}
	}

	return std::pair<ordering, bool>(toRet, false);
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

	size_t cand;

	std::pair<std::vector<double>, int> beat_data;
	beat_data.first.resize(input.get_num_candidates());

	for (cand = 0; cand < input.get_num_candidates(); ++cand) {
		if (!hopefuls[cand]) {
			continue;
		}

		for (size_t challenger = 0; challenger < input.get_num_candidates();
			++challenger) {

			if (cand != challenger && hopefuls[challenger])
				beat_data.first[challenger] = (input.
						get_magnitude(challenger, cand,
							hopefuls));
			else {
				beat_data.first[challenger] = 0;
			}
		}

		if (debug) {
			std::cout << "Before sorting: " << cand << "\t";
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

		beat_data.second = cand;
		scores.push_back(beat_data);
	}

	// Sort by least greatest score (if Minmax) or least least score
	// (if Minmin).
	sort(scores.begin(), scores.end());

	if (debug) {
		for (size_t cand = 0; cand < scores.size(); ++cand) {
			std::cout << "After sorting: " << scores[cand].second
				<< "\t";
			copy(scores[cand].first.begin(),
				scores[cand].first.end(),
				std::ostream_iterator<double>(std::cout, "   "));
			std::cout << std::endl;
		}
	}

	ordering to_ret;

	// Start adding to the output ordering
	int rank = 0;
	for (cand = 0; cand < scores.size(); ++cand) {
		// If it's not first rank and the previous score array is
		// different from this one, then we're worse than the previous
		// one, otherwise it's equal rank.
		if (cand != 0) {
			if (scores[cand].first != scores[cand-1].first) {
				--rank;
			}
		}

		to_ret.insert(candscore(scores[cand].second, rank));
	}

	return std::pair<ordering, bool>(to_ret, false);
}

// "Maxmin"
std::pair<ordering, bool> maxmin::pair_elect(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	ordering toRet;

	for (size_t candidate = 0; candidate < input.get_num_candidates();
		++candidate) {

		if (!hopefuls[candidate]) {
			continue;
		}

		size_t record_pos, first;
		bool seen_record = false, seen_first = false;

		for (size_t challenger = 0; challenger < input.get_num_candidates();
			++challenger) {

			if (!hopefuls[challenger]) {
				continue;
			}
			if (candidate == challenger) {
				continue;
			}
			if (!seen_first) {
				first = challenger;
				seen_first = true;
			}

			if ((!seen_record || input.get_magnitude(candidate,
						challenger, hopefuls) <
					input.get_magnitude(candidate,
						record_pos,
						hopefuls)) &&
				input.get_magnitude(candidate, challenger,
					hopefuls) != 0) {
				record_pos = challenger;
				seen_record = true;
			}
		}

		if (!seen_record) {
			record_pos = first;    // force 0 if loser
			seen_record = true;
		}

		toRet.insert(candscore(candidate, input.get_magnitude(candidate,
					record_pos)));

	}

	return std::pair<ordering, bool>(toRet, false);
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

std::vector<double> copeland::get_copeland(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls,
	const std::vector<double> & counterscores) const {

	std::vector<double> scores(input.get_num_candidates(), 0);

	for (size_t candidate = 0; candidate < input.get_num_candidates();
		++candidate) {

		if (!hopefuls[candidate]) {
			continue;
		}
		size_t wins = 0, ties = 0;

		for (size_t challenger = 0; challenger < input.get_num_candidates();
			++challenger) {

			if (!hopefuls[challenger]) {
				continue;
			}
			if (input.get_magnitude(candidate, challenger, hopefuls) >
				input.get_magnitude(challenger, candidate,
					hopefuls)) {
				wins += counterscores[challenger];
			} else if (input.get_magnitude(candidate, challenger, hopefuls) ==
				input.get_magnitude(challenger, candidate,
					hopefuls)) {
				ties += counterscores[challenger];
			}
		}

		scores[candidate] = wins * win + ties * tie;
	}

	return scores;
}

std::pair<ordering, bool> copeland::pair_elect(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls,
	cache_map * cache, bool winner_only) const {

	ordering toRet;

	// To make ordinary Copeland for the first round.
	std::vector<double> scores(input.get_num_candidates(), 1);

	for (unsigned int cur_order = 0; cur_order < order; ++cur_order) {
		scores = get_copeland(input, hopefuls, scores);
	}

	// Spool it all into the ordering
	for (size_t candidate = 0; candidate < input.get_num_candidates();
		++candidate) {
		if (hopefuls[candidate]) {
			toRet.insert(candscore(candidate, scores[candidate]));
		}
	}

	return std::pair<ordering, bool>(toRet, false);
}

// Schulze!

std::pair<ordering, bool> schulze::pair_elect(
	const abstract_condmat & input,
	const std::vector<bool> & hopefuls, cache_map * cache,
	bool winner_only) const {

	beatpath bpath(input, CM_PAIRWISE_OPP, hopefuls);

	// Count defeats.
	size_t i, j, numcand = bpath.get_num_candidates();
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

	return std::pair<ordering, bool>(social_ordering, false);
}
