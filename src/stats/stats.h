
#ifndef __VOTE_STATS
#define __VOTE_STATS

#include "../tools.h"
#include "confidence/student.h"
#include <iostream>

using namespace std;

// A statistics class for storing results (usually from voting methods, or
// "best of"/"worst of"). 

// BLUESKY?: Parameter to decide whether to permit sorted_scores. If not,
// get median is linear time every time, otherwise it's log(n). Doing it in
// a sorted manner pays off if we check the median more often than p times s.th.
// n log(n) + p log(n) < pn. I don't think that happens very often.

// The stats types are:
// 	MS_UNNORM = Unnormalized (just a plain average)
// 	MS_INTRAROUND = Each entry is normalized between best and worst: 
// 			a/b + c/d
// 	MS_INTERROUND = The sum is normalized between sum of best and sum of 
// 			worst: (a+c)/(b+d)

// Since the class is templated, the code has to be in the header. This
// surprised me, but isn't so strange in retrospect.

enum stats_type { MS_UNNORM, MS_INTRAROUND, MS_INTERROUND };

template<typename T> class stats {
	private:
		string name;
		vector<T> scores;
		vector<T> normalized_scores;

		int num_scores;    // if we opt not to save them all.

		T scores_sum;      // so that get_mean is const time
		T norm_scores_sum; // ditto

		T scores_sq_sum;   // Used for variance calculations
		T norm_scores_sq_sum; // Same thing.

		// For inter-round. See the constructor for a better explanation
		// of what each does.
		T sum_minimum, sum_maximum;
		T sq_sum_minimum, sq_sum_maximum;
		T sq_sum_normdiff;
		T sum_normdiff; // For quickly and stably calculating the mean.

		// For Pareto front generation when we're not really interested
		// in exactly which outcomes constitute the front.
		bool keep_only_sum;

		stats_type normalization;

		// SLOW!
		T calc_median(vector<T> in) const;

		// Private now because getting the median of INTRAROUND is not
		// possible if we're INTERROUND and vice versa, nor can you get
		// the median of those if you're neither (or vice versa). This
		// significantly cuts down on space required, and if you need
		// both INTERROUND and INTRAROUND, just use the cache to quickly
		// dump into two stats structures!
		T get_median(stats_type norm_type) const;

	protected:
		void initialize(stats_type norm_type, string name_in,
				bool only_sum);

	public:
		stats(stats_type norm_type, string name_in, bool only_sum);
		stats(stats_type norm_type, string name_in);

		void add_result(T minimum, T result, T maximum);

		void set_normalization(stats_type norm_type) { 
			normalization = norm_type; }

		stats_type get_normalization() const { return(normalization); }

		T get_mean(stats_type norm_type) const;
		T get_variance(stats_type norm_type) const;
		T get_last(stats_type norm_type) const;

		T get_mean() const {return(get_mean(normalization));}
		T get_variance() const { return(get_variance(normalization)); }
		T get_median() const { return(get_median(normalization)); }
		T get_last() const { return(get_last(normalization)); }

		double get_confidence_interval(double uncertainty, 
				t_confidence_int & confidence_checker) const;

		string display_stats(bool show_median, double interval) const;
		virtual string get_name() const { return(name); }
};

template <typename T> T stats<T>::calc_median(vector<T> in) const {

	// If the number of members isn't divisible by two, then pick the
	// middle (floor(x/2)+1). Since we index from 0, it's just floor(x/2).
	// Similarly, if it's even, it's the mean of floor(x/2)-1 and 
	// floor(x/2).
	
	if (in.size() % 2 == 0) {
		typename vector<T>::iterator fpos = in.begin() + 
			(in.size() / 2) - 1, spos = fpos + 1;

		nth_element(in.begin(), fpos, in.end());
		nth_element(in.begin(), spos, in.end());
		return ((*fpos + *spos) * 0.5);
	}

	typename vector<T>::iterator pos = in.begin() + 
		(int)floor(in.size() * 0.5);
	nth_element(in.begin(), pos, in.end());
	return(*pos);
}


template <typename T> void stats<T>::initialize(stats_type norm_type, 
		string name_in, bool only_sum) {

	normalization = norm_type;
	name = name_in;
	scores_sum = 0;
	scores_sq_sum = 0;
	norm_scores_sum = 0;
	norm_scores_sq_sum = 0;
	num_scores = 0;
	keep_only_sum = only_sum;

	// In interround mode, the "mean" is (sum of values - sum of minima)/
	// (sum of maxima - sum of minima). Like the intraround mean, it has
	// a range of 0...1.

	// To calculate mean and variance in an online manner, we make use of
	// the following observations: 
	// scores_sum - sum_min        n     scores[k] - minimum[k]
	// --------------------  =    SUM    -----------------------
	//  sum_max - sum_min         k=0       sum_max - sum_min
	//
	// and: (scores_sum - sum_min) = SUM (k=0..n) scores[k] - minimum[k]).
	//
	// So we add, for each round k, its score minus that round's minimum, 
	// to sum_normdiff, for calculating the mean, and the square of that,
	// for calculating the variance, to sq_sum_normdiff.

	// There is another trick. Note that the above expression is for the
	// *mean*. But the mean is divided by the number of entries (rounds)
	// so far, so in order to make that cancel out, the denominator has
	// to be (sum_max - sum_min) * 1/n. This becomes important when
	// calculating the variance; in get_mean(), we just don't divide by n
	// so no compensation is required.

	// TODO? Perhaps use this insight to make better medians for interround?

	sum_minimum = 0; 
	sum_maximum = 0;

	sq_sum_minimum = 0;
	sq_sum_maximum = 0;

	sq_sum_normdiff = 0;
	sum_normdiff = 0;
	num_scores = 0;
	keep_only_sum = only_sum;
}

template <typename T> stats<T>::stats(stats_type norm_type,
		string name_in, bool only_sum) {
	initialize(norm_type, name_in, only_sum);
}

template <typename T> stats<T>::stats(stats_type norm_type, string name_in) {
	initialize(norm_type, name_in, false);
}

template <typename T> void stats<T>::add_result(T minimum, T result, 
		T maximum) {

	T normalized = renorm(minimum, maximum, result, (T)0.0, (T)1.0);

	if (!keep_only_sum) {
		scores.push_back(result);
		if (get_normalization() == MS_INTRAROUND)
			normalized_scores.push_back(normalized);
		else	
			if (get_normalization() == MS_INTERROUND)
				normalized_scores.push_back(result - minimum);
	}

	++num_scores;
	scores_sum += result;
	scores_sq_sum += result * result;
	sum_minimum += minimum;
	sq_sum_minimum += (minimum * minimum);
	sum_maximum += maximum;
	sq_sum_maximum += (maximum * maximum);
	norm_scores_sum += normalized;
	norm_scores_sq_sum += (normalized * normalized);
	sq_sum_normdiff += square(result - minimum);
	sum_normdiff += (result - minimum);

}

template <typename T> T stats<T>::get_mean(stats_type norm_type) const {
	switch(norm_type) {
		case MS_UNNORM:
			return(scores_sum / (T)num_scores);
		case MS_INTRAROUND:
			return(norm_scores_sum / (T)num_scores);
		case MS_INTERROUND:
			// For great numerical stability!
			return(sum_normdiff / (sum_maximum - sum_minimum));
		default: assert (1 != 1); // Shouldn't happen.
	}
}

// This is the sample variance, i.e. an unbiased estimator of the population
// variance.

// And TODO: Fix the minus zeroes.

template <typename T> T stats<T>::get_variance(stats_type norm_type) const {
	// Here we make use of the computational algorithm for the sample
	// variance: 
	// (sigma hat)^2 = N / (N-1) * ( 1/N square_sum - square(mean))

	// For interround, if the denominator "denom" is
	// (sum_max - sum_min) / num_scores, then we know the sum is
	// SUM 0...n (val[n] - min[n]) / denom, and the square sum is then
	// (SUM 0...n (val[n] - min[n])^2) / denom^2, and the numerator term
	// is equal to sq_num_normdiff.

	T square_sum, square_mean = square(get_mean(norm_type)),
	  adj_denominator = (sum_maximum - sum_minimum) / (T)num_scores;

	switch(norm_type) {
		case MS_UNNORM:
			square_sum = scores_sq_sum;
			break;
		case MS_INTRAROUND:
			square_sum = norm_scores_sq_sum;
			break;
		case MS_INTERROUND:
			square_sum = sq_sum_normdiff / square(adj_denominator);
			break;
		default: assert (1 != 1);
	}

	// N / (N-1) * ( 1/N square_sum - square(mean))
	return( num_scores / (T)(num_scores-1) * ( square_sum/(T)num_scores
				- square_mean));
}

// Can this be made summable? Nope! (Not short of bucketing.)

template <typename T> T stats<T>::get_median(stats_type norm_type) const {
	assert(!keep_only_sum);
	if (norm_type != get_normalization()) return(-INFINITY);

	switch(norm_type) {
		case MS_UNNORM:
			return(calc_median(scores));
		case MS_INTRAROUND:
			return(calc_median(normalized_scores));
		case MS_INTERROUND:
			// We now use the new formulation (see the constructor).
			return(calc_median(normalized_scores) /
					((sum_maximum - sum_minimum) / (T)
					 num_scores));
		default: assert(1 != 1); // Shouldn't happen either.
	}
}

template <typename T> T stats<T>::get_last(stats_type norm_type) const {
	assert (!keep_only_sum);
	assert (!normalized_scores.empty() && !scores.empty());

	switch(norm_type) {
		case MS_UNNORM:
			return(*scores.rbegin());
		case MS_INTRAROUND:
			return(*normalized_scores.rbegin());
		case MS_INTERROUND:
			return(renorm(sum_minimum / (T)scores.size(),
						sum_maximum/(T)
						scores.size(),
						*scores.rbegin(), (T)0.0, 
						(T)1.0));
		default: assert (1 != 1);
	}
}

template<typename T> double stats<T>::get_confidence_interval(
		double uncertainty, t_confidence_int & 
		confidence_checker) const {

	return(confidence_checker.interval(get_mean(), get_variance(), 
				num_scores, uncertainty));
}

// If show_median is true, we show the mean and median, otherwise we show the
// mean and its confidence interval.
template<typename T> string stats<T>::display_stats(bool show_median,
		double interval) const {

	t_confidence_int x;

	string stats;
	if (show_median)
		stats = s_padded(dtos(get_mean(), 5), 7) + " med: " +
				s_padded(dtos(get_median(), 5), 7);
	else {
		// If the user specified a value greater than 50%, he probably
		// means a confidence of that value instead of a p, so correct.
		double uncertainty;
		if (interval >= 0.5)
			uncertainty = 1 - interval;
		else	uncertainty = interval;

		stats = s_padded(dtos(get_mean(), 5), 7) + "  +/- " +
			s_padded(dtos(get_confidence_interval(uncertainty, x),
						5), 7);
	}

	return(stats + "         " + s_padded(get_name(), 50));
}

#endif
