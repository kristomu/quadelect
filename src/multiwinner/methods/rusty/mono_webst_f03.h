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

#include "set_functs.h"

#include "hack/msvc_random.h"

enum margin_type_f03 { MMF_PLUSONE = 0, MMF_PLUSHALF = 1, MMF_FABS = 2,
	MMF_INVERTED = 3, MMF_UNCERTAIN = 4, MMF_UNCERTAIN_INF = 5
};

class mono_webster_f03 : public multiwinner_method {

	private:
		set_functions sf;
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

		margin_type_f03 margin_management;
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
		mono_webster_f03(bool limit_in, margin_type_f03 marg_man,
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
			string name_out = "M-Set Webster (f03, EXP, " +
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