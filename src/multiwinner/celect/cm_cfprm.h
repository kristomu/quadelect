// Ya ya QnD

// BLUESKY: Support other positional systems.

#ifndef __CM_CFPRM
#define __CM_CFPRM

#include "ballot_struct.h"
#include <vector>
#include <iostream>
#include <math.h>
#include <string>

#include "comparison.h"

// PROPORTIONAL REPRESENTATION - CFPRM

class CFPRM : public ComparisonMethod<std::vector<ballot_bunch> > {

	private:
		bool debug;
		int candidates;

		double first_score;
		double second_score;

		std::vector<ballot_bunch> voting_record; // See CPO-STV.

		void set_data(int candidates_in, const std::vector
			<ballot_bunch> & data_in);
		void post_initialize(bool debug_in);

		// the meat: internal functions

		std::vector<double> get_borda_sum(const
			std::vector<ballot_bunch> & box,
			double & num_voters);

		int favors_A(const std::vector<bool> & members_of_A,
			const std::vector<bool> & members_of_B,
			const ballot_bunch & this_ballot,
			double & r_favored);

		double get_rab_borda(const std::vector<double> & rank_sum,
			const std::vector<bool> & members_of_A,
			const std::vector<bool> & members_of_B,
			const double & num_voters);

		int get_weight(const ballot_bunch & this_ballot,
			const double & rab, const std::vector<bool> &
			members_of_A, const std::vector<bool> &
			members_of_B);

		void get_values(const std::vector<bool> & members_of_A,
			const std::vector<bool> & members_of_B,
			const std::vector<ballot_bunch> & box, double &
			a_results, double & b_results);

	public:

		// I will give you two. (No, I want half or a duel for all.)
		void init(int candidates_in, const std::vector<ballot_bunch> &
			data_in);
		CFPRM(int candidates_in, const std::vector<ballot_bunch> &
			data_in);

		void compare(const std::vector<bool> & first_set, const
			std::vector <bool> & second_set);
		double get_first_score();
		double get_second_score();
		std::string get_identity(bool long_form);
};

#endif //__CM_CFPRM

