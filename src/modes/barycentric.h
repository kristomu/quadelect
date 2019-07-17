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

#include <vector>
#include <string>
#include <openssl/sha.h>

using namespace std;

class barycentric : public mode {

	private:
		vector<const election_method *> e_methods;
		vector<vector<double> > cand_colors;

		list<ballot_group>  generate_ballot_set(double x, double y,
				double maxvoters, string first_group,
				string sec_group, string third_group,
				double x_1, double y_1, double x_2,
				double y_2, double x_3, double y_3) const;

		// Returns empty list if it's not inside the triangle.
		list<ballot_group> generate_ballot_set(double x, double y,
				double maxvoters) const;

		vector<vector<double> > get_candidate_colors(int numcands,
		                bool debug) const;

		string get_sha_code(const election_method & in,
				size_t bytes) const;

		bool inited;
		int max_rounds, cur_round;

	public:

		barycentric() { inited = false; }

		bool init(rng & randomizer);
		int get_max_rounds() const;
		int get_current_round() const;

		string do_round(bool give_brief_status, bool reseed,
				rng & randomizer);

		vector<string> provide_status() const;

		// Add methods.
		void add_method(const election_method * to_add);
		template<typename T> void add_methods(T start_iter, T end_iter);
};

template<typename T> void barycentric::add_methods(T start_iter, T end_iter) {

	for (T iterator = start_iter; iterator != end_iter; ++iterator)
		add_method(*iterator);
}
