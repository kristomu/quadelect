#pragma once

// A possible monotone version of Set Webster. It's a huge mess, but
// that will change if it's actually monotone.

#include <iostream>
#include <algorithm>
#include <set>
#include <list>
#include <iterator>
#include <vector>
#include <math.h>
#include <assert.h>

#include "auxiliary/dsc.h"
#include "multiwinner/methods/methods.h"
#include "tools/ballot_tools.h"
#include "setwise/coalition.h"
#include "tools/tools.h"

#include "hack/msvc_random.h"

enum margin_type_c37 { MMC_PLUSONE = 0, MMC_PLUSHALF = 1, MMC_FABS = 2,
	MMC_INVERTED = 3, MMC_UNCERTAIN = 4, MMC_UNCERTAIN_INF = 5
};

class mono_webster_c37 : public multiwinner_method {

	private:
		solid_coalition construct_sc(string memberset,
			double support) const;
		string show_membership(const std::set<int> & x) const;
		string show_membership(const solid_coalition & x) const;
		void power_set(int numcands, int curcand, std::set<int> current,
			std::set<int> remaining,
			std::set<std::set<int> > & power_set_out) const;
		void choose_set(int numcands, int curcand, int how_many,
			int cur_chosen, std::set<int> & current,
			std::set<std::set<int> > & choose_out) const;
		double get_retained(const solid_coalition & check_against,
			const std::set<int> & proposed) const;
		int share_candidates(const std::set<int> & smaller,
			const std::set<int> & larger,
			int early_break) const;
		int share_candidates(const solid_coalition & check_against,
			const std::set<int> & proposed,
			int early_break) const;
		bool verify(const std::vector<solid_coalition> & constraints,
			const std::set<int> & proposed,
			double divisor) const;
		void explain(const std::vector<solid_coalition> & constraints,
			double divisor) const;
		std::set<std::set<int> > get_passed_sets(const std::vector<solid_coalition>
			&
			constraints, const std::set<std::set<int> > & in,
			double divisor) const;
		std::set<std::set<int> > get_passed_lowest(const
			std::vector<solid_coalition> & constraints,
			const std::set<std::set<int> > & in, double high,
			double low, double stepsize,
			double & divisor) const;
		std::vector<double> alt_voter_support(const
			std::vector<solid_coalition> & constraints,
			const std::set<int> & current,
			const std::vector<double> & tiebreak_array,
			double divisor) const;
		std::set<std::set<int> > get_highest_scoring_tiebreak(
			const std::vector<solid_coalition> & constraints,
			const std::set<std::set<int> > & input,
			const std::vector<double> & tiebreak_array,
			double divisor) const;
		std::set<int> naive_sweb(const std::vector<solid_coalition> & constraints,
			int num_seats, double stepsize) const;

		margin_type_c37 margin_management;
		bool simple_membership;
		bool debug;
		bool limiter;

	public:
		// Debug functions.
		std::set<int> get_brute_force_pure_ballot(int num_seats,
			double stepsize, int a, int b, int c, int d,
			int e, int f) const;
		std::set<int> check_one_brute() const;

		// You know why these are here...

		// TODO: typedef enum
		mono_webster_c37(bool limit_in, margin_type_c37 marg_man,
			bool simple_member, bool debug_in) {
			limiter = limit_in;
			margin_management = marg_man;
			simple_membership = simple_member;
			debug = debug_in;
		}


		council_t get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		string name() const {
			// Should mention debug here?
			string name_out = "M-Set Webster (c37, EXP, " +
				itos((int)margin_management);
			if (simple_membership) {
				name_out += ", simple, ";
			} else	{
				name_out += ", complx, ";
			}
			if (limiter) {
				name_out += "limiter)";
			} else	{
				name_out += "unlimit)";
			}

			return (name_out);
		}
};