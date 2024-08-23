#pragma once

#include "tools/tools.h"
#include "multiwinner/methods.h"

#include <memory>

// Ugly hack proceeds. We really need to separate this into two classes, one
// for the method and one for stats.

class multiwinner_stats {
	private:
		std::shared_ptr<multiwinner_method> mw_method;
		std::string name;
		std::vector<double> scores;
		std::vector<double> normalized_scores;

		double scores_sum; // so that get_average is const time
		double norm_scores_sum; // ditto

		double get_median(std::vector<double> & in) const;

	public:
		multiwinner_stats(std::shared_ptr<multiwinner_method> method_in);
		multiwinner_stats(std::string meta_name);

		//~multiwinner_stats();

		const std::shared_ptr<multiwinner_method> method() {
			return mw_method;
		}

		void add_result(double minimum, double result, double maximum);

		double get_average(bool normalized) const;
		double get_median(bool normalized);
		double get_last(bool normalized) const;

		std::string display_stats();
		std::string get_name() const {
			return (name);
		}
};