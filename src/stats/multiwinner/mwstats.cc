
#include "mwstats.h"
#include <set>
#include <iostream>

double multiwinner_stats::get_median(std::vector<double> & in) const {

	// If the number of members isn't divisible by two, then pick the
	// middle (floor(x/2)+1). Since we index from 0, it's just floor(x/2).
	// Similarly, if it's even, it's the mean of floor(x/2)-1 and floor(x/2)
	// .

	if (in.size() % 2 == 0) {
		std::vector<double>::iterator fpos = in.begin() + (in.size() / 2)
			- 1, spos = fpos + 1;

		nth_element(in.begin(), fpos, in.end());
		nth_element(in.begin(), spos, in.end());
		return ((*fpos + *spos) * 0.5);
	}

	std::vector<double>::iterator pos = in.begin() + (int)floor(
			in.size() * 0.5);
	nth_element(in.begin(), pos, in.end());
	return (*pos);
}

multiwinner_stats::multiwinner_stats(
	std::shared_ptr<multiwinner_method> method_in) {

	mw_method = method_in;
	name = method_in->name();
	scores_sum = 0;
	norm_scores_sum = 0;
}

// Hack for "meta-methods" that pick best-of, average-of, worst-of.
multiwinner_stats::multiwinner_stats(std::string meta_name) {
	mw_method = NULL;
	name = meta_name;
	scores_sum = 0;
	norm_scores_sum = 0;
}

void multiwinner_stats::add_result(double minimum, double result,
	double maximum) {

	double normalized = renorm(minimum, maximum, result, 0.0, 1.0);

	scores.push_back(result);
	normalized_scores.push_back(normalized);

	scores_sum += result;
	norm_scores_sum += normalized;
}

double multiwinner_stats::get_average(bool normalized) const {
	// Do assert here to check if these are the same. Bug suggests they
	// are not; there's a case where average > median! Or meh, seems
	// that's possible.
	/*	const std::vector<double> * rel;

		if (normalized) {
			rel = &normalized_scores;
		} else {
			rel = &scores;
		}

		double total = 0; int count = 0;

		for (int counter = 0; counter < rel->size(); ++counter)
			total += (*rel)[counter];

		return(total/(double)(rel->size()));*/
	if (normalized) {
		return (norm_scores_sum / (double)normalized_scores.size());
	} else	{
		return (scores_sum / (double)scores.size());
	}
}

double multiwinner_stats::get_median(bool normalized) {
	if (normalized) {
		return (get_median(normalized_scores));
	} else	{
		return (get_median(scores));
	}
}

double multiwinner_stats::get_last(bool normalized) const {
	assert(!normalized_scores.empty() && !scores.empty());
	if (normalized) {
		return (*normalized_scores.rbegin());
	} else	{
		return (*scores.rbegin());
	}
}

std::string multiwinner_stats::display_stats() {

	/*std::string name;

	if (mw_method == NULL)
		name = "N/A";
	else	name = mw_method->name();*/

	return (s_padded(dtos(get_average(true), 5), 7) + " " +
			s_padded(dtos(get_median(true), 5), 7) + "  " +
			s_padded(name, 32) + "round: " + s_padded(dtos(
					get_last(true), 5), 7) + " (unnorm: " +
			s_padded(dtos(get_last(false), 3), 5) + ")");
}

// TEST
/*
main() {

	multiwinner_stats p(NULL);

	p.add_result(0, 0, 10);
	p.add_result(0, 1, 10);

	cout << "Median: " << p.get_median(false) << endl; // 0.5

	p.add_result(0, 2, 10);

	cout << "Median: " << p.get_median(false) << endl; // 1

	p.add_result(0, 3, 10);
	cout << "Median: " << p.get_median(false) << endl; // 1.5

	p.add_result(0, 4, 10);
	cout << "Median: " << p.get_median(false) << endl; // 2
}*/