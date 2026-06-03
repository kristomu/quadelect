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

enum margin_type { MM_PLUSONE = 0, MM_PLUSHALF = 1, MM_FABS = 2, MM_INVERTED = 3 };

class mono_webster_640 : public multiwinner_method {

	private:
		solid_coalition construct_sc(string memberset,
			double support) const;
		string show_membership(const std::set<int> & x) const;
		string show_membership(const solid_coalition & x) const;
		void power_set(int numcands, int curcand, std::set<int> current,
			std::set<int> remaining,
			std::set<std::set<int> > & power_set_out) const;
		double get_retained(const solid_coalition & check_against,
			const std::set<int> & proposed) const;
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

		margin_type margin_management;
		bool simple_membership;
		bool debug;

	public:
		// Debug functions.
		std::set<int> get_brute_force_pure_ballot(int num_seats,
			double stepsize, int a, int b, int c, int d,
			int e, int f) const;
		std::set<int> check_one_brute() const;

		// You know why these are here...

		// TODO: typedef enum
		mono_webster_640(margin_type marg_man, bool simple_member, bool debug_in) {
			margin_management = marg_man;
			simple_membership = simple_member;
			debug = debug_in;
		}


		council_t get_council(size_t council_size, size_t num_candidates,
			const election_t & ballots) const;

		string name() const {
			// Should mention debug here?
			string name_out = "M-Set Webster (640, EXP, " +
				itos((int)margin_management);
			if (simple_membership) {
				name_out += ", simple)";
			} else	{
				name_out += ", complx)";
			}
			return (name_out);
		}
};