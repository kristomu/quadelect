
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
	random_sum = 0;
	optimal_sum = 0;
}

// Hack for "meta-methods" that pick best-of, average-of, worst-of.
multiwinner_stats::multiwinner_stats(std::string meta_name) {
	mw_method = NULL;
	name = meta_name;
	scores_sum = 0;
	random_sum = 0;
	optimal_sum = 0;
}

void multiwinner_stats::add_result(double random_score,
	double result_score, double optimal_score) {

	assert((random_score <= optimal_score && result_score <= optimal_score)
		|| (random_score >= optimal_score && result_score >= optimal_score));

	scores.push_back(result_score);

	scores_sum += result_score;
	random_sum += random_score;
	optimal_sum += optimal_score;

	last_random = random_score;
	last_optimum = optimal_score;
}

double multiwinner_stats::get_average(bool normalized) const {

	double num_scores = scores.size();

	if (normalized) {
		// (E[method]-E[random])/(E[optimal]-E[random]
		return renorm(random_sum, optimal_sum, scores_sum, 0.0, 1.0);
	} else	{
		return scores_sum / num_scores;
	}
}

double multiwinner_stats::get_last(bool normalized) const {
	assert(!scores.empty());

	double last_score = *scores.rbegin();

	if (normalized) {
		return renorm(last_random, last_optimum, last_score, 0.0, 1.0);
	} else	{
		return last_score;
	}
}

std::string multiwinner_stats::display_stats(
	double additional_stat) const {

	std::string additional_stat_str;
	if (isfinite(additional_stat)) {
		additional_stat_str = s_padded(dtos(additional_stat, 5), 7);
	} else {
		additional_stat_str = "?????";
	}

	return (s_padded(dtos(get_average(true), 5), 8) + " " +
			additional_stat_str + "  " +
			s_padded(name, 32) + "round: " + s_padded(dtos(
					get_last(true), 4), 7) + " (unnorm: " +
			s_padded(dtos(get_last(false), 3), 5) + ")");
}

std::string multiwinner_stats::display_stats() const {

	return display_stats(NAN);
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