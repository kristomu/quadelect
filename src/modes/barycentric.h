// Barycentric coordinates vote "ID". This draws a triangle that identifies the
// particular voting method: each three-candidate election summing up to a
// constant number of voters has a point in the triangle.

// PPM format. Probably could share a lot of code with Yee. Do that later.

// We'll separate out the barycentric generator later.

#pragma once

#include "mode.h"
#include "../ballots.h"
#include "../random/random.h"
#include "../images/color/color.h"
#include "../singlewinner/method.h"

#include <memory>
#include <vector>
#include <string>

class barycentric : public mode {

	private:
		std::vector<std::shared_ptr<const election_method> > e_methods;
		std::vector<std::vector<double> > cand_colors;

		election_t  generate_ballot_set(double x, double y,
			double maxvoters, std::string first_group,
			std::string sec_group, std::string third_group,
			double x_1, double y_1, double x_2,
			double y_2, double x_3, double y_3) const;

		// Returns empty list if it's not inside the triangle.
		election_t generate_ballot_set(double x, double y,
			double maxvoters) const;

		std::vector<std::vector<double> > get_candidate_colors(int numcands,
			bool debug) const;

		std::string get_codename(const election_method & in,
			size_t bytes) const;

		bool is_valid_purpose(uint32_t purpose) const {
			return false;
		}

		bool inited;
		int max_rounds, cur_round;

	public:

		barycentric() {
			inited = false;
		}

		bool init();
		int get_max_rounds() const;
		int get_current_round() const;

		std::string do_round(bool give_brief_status);

		std::vector<std::string> provide_status() const;

		// Add methods.
		void add_method(std::shared_ptr<const election_method > to_add);
		template<typename T> void add_methods(T start_iter, T end_iter);

		std::string name() const {
			return "Barycentric";
		}
};

template<typename T> void barycentric::add_methods(T start_iter,
	T end_iter) {

	for (T iterator = start_iter; iterator != end_iter; ++iterator) {
		add_method(*iterator);
	}
}
