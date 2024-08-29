#pragma once

#include "tools/tools.h"
#include "multiwinner/methods.h"

#include <memory>

// Ugly hack proceeds. We really need to separate this into two classes, one
// for the method and one for stats.

// Let's hack it a little bit to get something closer to VSE. Then, later, I
// should use actual VSE.

class multiwinner_stats {
	private:
		std::shared_ptr<multiwinner_method> mw_method;
		std::string name;
		std::vector<double> scores;

		double scores_sum; // so that get_average is const time
		double random_sum;
		double optimal_sum;

		double last_random, last_optimum;

		double get_median(std::vector<double> & in) const;

	public:
		multiwinner_stats(std::shared_ptr<multiwinner_method> method_in);
		multiwinner_stats(std::string meta_name);

		//~multiwinner_stats();

		const std::shared_ptr<multiwinner_method> method() {
			return mw_method;
		}

		void add_result(double random_score,
			double result_score, double optimal_score);

		double get_average(bool normalized) const;
		//double get_median(bool normalized);
		double get_last(bool normalized) const;

		std::string display_stats();
		std::string get_name() const {
			return (name);
		}
};