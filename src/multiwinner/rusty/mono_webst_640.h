#pragma once

// A possible monotone version of Set Webster. It's a huge mess, but that will
// change if it's actually monotone.

#include <iostream>
#include <algorithm>
#include <set>
#include <list>
#include <iterator>
#include <vector>
#include <math.h>
#include <assert.h>

#include "auxiliary/dsc.h"
#include "multiwinner/methods.h"
#include "tools/ballot_tools.h"
#include "setwise/coalition.h"
#include "tools/tools.h"

using namespace std;

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


		std::list<size_t> get_council(size_t council_size, size_t num_candidates,
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


// Perhaps these should be in coalition.cc

solid_coalition mono_webster_640::construct_sc(string memberset,
	double support) const {
	solid_coalition output;

	for (size_t counter = 0; counter < memberset.size(); ++counter) {
		output.candidates.insert(memberset[counter]-'A');
	}

	output.support = support;
	return (output);
}

string mono_webster_640::show_membership(const std::set<int> & x) const {
	string out;

	for (std::set<int>::const_iterator p = x.begin(); p != x.end(); ++p) {
		out += 'A' + *p;
	}

	return (out);
}

string mono_webster_640::show_membership(const solid_coalition & x) const {
	return (show_membership(x.candidates));
}

void mono_webster_640::power_set(int numcands, int curcand,
	std::set<int> current,
	std::set<int> remaining, std::set<std::set<int> > & power_set_out) const {

	if (curcand == numcands) {
		power_set_out.insert(current);
		return;
	} else {
		std::set<int>::iterator pos = remaining.begin(), bakpos;
		while (pos != remaining.end()) {
			std::set<int> newcurr = current, newrema = remaining;
			newcurr.insert(*pos);
			newrema.erase(newrema.find(*pos));
			power_set(numcands, curcand+1, newcurr, newrema,
				power_set_out);
			++pos;
		}
	}
}

double mono_webster_640::get_retained(const solid_coalition &
	check_against,
	const std::set<int> & proposed) const {

	// ... intersect
	std::set<int> intersect;

	set_intersection(check_against.candidates.begin(),
		check_against.candidates.end(),
		proposed.begin(), proposed.end(), inserter(
			intersect, intersect.begin()));

	// ... get size
	return (intersect.size());
}

bool mono_webster_640::verify(const std::vector<solid_coalition> &
	constraints,
	const std::set<int> & proposed, double divisor) const {

	for (size_t counter = 0; counter < constraints.size(); ++counter) {
		int seats_deserved = round(constraints[counter].support /
				divisor);

		int actual = get_retained(constraints[counter], proposed);

		if (actual < seats_deserved) {
			return (false);
		}
	}
	return (true);
}

void mono_webster_640::explain(const std::vector<solid_coalition> &
	constraints,
	double divisor) const {

	cout << "Showing constraints for this divisor:" << endl;
	cout << "\tSet:\t#v:\tunround:\tat least:" << endl;

	for (size_t counter = 0; counter < constraints.size(); ++counter) {
		double unround = constraints[counter].support / divisor;

		cout << "\t" << show_membership(constraints[counter].candidates) << "\t" <<
			constraints[counter].support << "\t" << unround << "\t"
			<< round(unround) << "\t" << (floor(unround) + 0.5) - unround << endl;
	}
}

std::set<std::set<int> > mono_webster_640::get_passed_sets(
	const std::vector<solid_coalition> & constraints,
	const std::set<std::set<int> > & in, double divisor) const {
	std::set<std::set<int> > toRet;

	for (std::set<std::set<int> >::const_iterator spos = in.begin();
		spos != in.end();
		++spos) {
		if (verify(constraints, *spos, divisor)) {
			toRet.insert(*spos);
		}
	}

	return (toRet);
}

std::set<std::set<int> > mono_webster_640::get_passed_lowest(
	const std::vector<solid_coalition> & constraints,
	const std::set<std::set<int> > & in, double high, double low,
	double stepsize, double & divisor) const {
	// Find the lowest divisor possible where at least one set passes.
	// Uses binary search because I can't be bothered.
	double last_true = INFINITY;

	while (fabs(high - low) > stepsize) {
		double mid = low + ((high - low) / 2);
		int num_passed_sets = get_passed_sets(constraints, in,
				mid).size();
		//cout << "Trying <" << high << ", ["<< mid << "], " << low << ">: "<< num_passed_sets << endl;

		if (num_passed_sets > 0) {
			high = mid;
			last_true = mid;
		} else	{
			low = mid;
		}
	}

	assert(finite(last_true));

	std::set<std::set<int> > passed_sets = get_passed_sets(constraints, in,
			last_true);

	// DEBUG
	if (debug) {
		cout << "Binary search found divisor " << last_true << " which gives " <<
			passed_sets.size() << " solutions. " << endl;
		cout << "Solutions:";


		for (std::set<std::set<int> >::const_iterator pos = passed_sets.begin();
			pos != passed_sets.end(); ++pos) {
			cout << " {" << show_membership(*pos) << "}";
		}
		cout << endl;
		explain(constraints, last_true);
	}
	divisor = last_true;

	if (finite(last_true)) {
		return (passed_sets);
	} else	{
		return (std::set<std::set<int> >());    // Shouldn't happen
	}
}

// DONE: Somehow make this leximax. Just dump in all the margins where we do
// pass the requirement (includes), then do a "straightforward" leximax compare
// later.
// 	I don't know if it's proper leximax, but "v[0] < w[0]" then as
// 	tiebreaker, v[1] < w[1], then as tiebreaker... etc.

std::vector<double> mono_webster_640::alt_voter_support(
	const std::vector<solid_coalition> & constraints,
	const std::set<int> & current, const std::vector<double> & tiebreak_array,
	double divisor) const {

	//List is better, but we're not overly concerned with speed at the
	//moment.
	std::vector<double> retval;

	for (size_t counter = 0; counter < constraints.size(); ++counter) {
		double unround = constraints[counter].support / divisor;
		double margin = (floor(unround) + 0.5) - unround;

		// You can put what you want here and it doesn't break
		// monotonicity, it seems. Setting different forms of
		// adjustment here does alter the outcome in some cases.
		// Other adjustments that have also been tried:
		// margin = fabs(margin)
		//while (margin < 0) margin ++;
		while (margin < 0) {
			switch (margin_management) {
				default:
				case MM_PLUSONE: ++margin; break;
				case MM_PLUSHALF: margin += 0.5; break;
				case MM_FABS: margin = fabs(margin); break;
				case MM_INVERTED: margin = 1 + fabs(margin);
					break;
			};
		}

		//cout << "MARGIN: " << margin << "\t" << divisor << endl;

		bool permitted_this_score;
		// If simple membership, we can add the margin if we share
		// a candidate with the constraint. If not, the decision is
		// somewhat more complex. The former seems to avoid some
		// monotonicity errors, but we want to see how it works, first.
		// TODO: check council_size=1 cases.
		if (simple_membership)
			permitted_this_score = (get_retained(constraints
						[counter], current) > 0);
		else
			permitted_this_score = includes(constraints[counter].
					candidates.begin(),
					constraints[counter].candidates.end(),
					current.begin(), current.end())
				||
				includes(current.begin(), current.end(),
					constraints[counter].candidates.begin(),
					constraints[counter].candidates.end());

		// Idea: tiebreak also with number of candidates in common.
		// Either that as a leximax after the leximax, or as the pair,
		// e.g (-0.1,20) beats (-0.1,10), rather than tying it.
		if (permitted_this_score) {
			retval.push_back(margin);
		}
	}

	sort(retval.begin(), retval.end()); // Least is now first.

	return (retval);
}

std::set<std::set<int> > mono_webster_640::get_highest_scoring_tiebreak(
	const std::vector<solid_coalition> & constraints,
	const std::set<std::set<int> > & input,
	const std::vector<double> & tiebreak_array, double divisor) const {

	// First dump into a set to get sorted data.
	std::vector<pair<std::vector<double >, std::set<int> > > first_sort;

	for (std::set<std::set<int> > ::const_iterator pos = input.begin(); pos !=
		input.end(); ++pos) {
		if (debug) {
			cout << "Tiebreak: Considering " << show_membership(*pos) << "\t" <<
				flush;
		}
		std::vector<double> comp = alt_voter_support(constraints, *pos,
				tiebreak_array, divisor);
		if (debug) {
			copy(comp.begin(), comp.end(), ostream_iterator<double>(cout, " "));
			cout << endl;
		}

		first_sort.push_back(pair<std::vector<double >, std::set<int> >(comp,
				*pos));
	}

	// Sort. Since the individual doubles were sorted, this works as a kind
	// of leximax comparison: v[0] vs q[0], if they're equal, v[1] vs q[1],
	// etc.
	sort(first_sort.begin(), first_sort.end());
	//reverse(first_sort.begin(), first_sort.end());

	// Now get all the sets who are equal to the first as far as the record
	// goes (because those are best).
	//cout << "Verify: " << first_sort[0].first[0] << "\t" << first_sort[0].first[1] << endl;

	std::set<std::set<int> > toRet;

	bool eligible = true;
	for (std::vector<pair<std::vector<double>, std::set<int> > >::const_iterator
		spos =
			first_sort.begin(); spos != first_sort.end() &&
		eligible; ++spos) {
		cout << "Eligible: " << show_membership(spos->second) << endl;
		toRet.insert(spos->second);

		// Check if the next is also eligible.
		std::vector<pair<std::vector<double>, std::set<int> > >::const_iterator
		next;
		next = spos;
		++next;
		if (next != first_sort.end()) {
			eligible = (spos->first == next->first);
		}
	}

	return (toRet);
}

// Generate all possible councils of this size, then run tiebreak to find which
// we prefer.
std::set<int> mono_webster_640::naive_sweb(const
	std::vector<solid_coalition> & constraints,
	int num_seats, double stepsize) const {

	std::set<std::set<int> > psout;
	power_set(num_seats, 0, std::set<int>(), constraints[0].candidates, psout);
	cout << "Engaging divisor engine." << endl;
	double divisor;
	std::vector<double> tiebreak_array(0); // NOT TO BE ACCESSED

	std::set<std::set<int> > accepted_limiters = get_passed_lowest(constraints,
			psout, constraints[0].support * 4, 1, stepsize,
			divisor);

	// Now we have a bunch of sets.
	//cout << "Sets after divisor stuff: " << accepted_limiters.size() << endl;
	if (accepted_limiters.size() == 1) {
		return (*accepted_limiters.begin());
	} else {
		cout << "Time for breaking some ties, methinks." << endl;
		accepted_limiters = get_highest_scoring_tiebreak(constraints,
				accepted_limiters, tiebreak_array, divisor);
	}

	cout << "Now there are " << accepted_limiters.size() << " sets. ";
	if (accepted_limiters.size() > 1) {
		cout << "Breaking randomly.";
	}
	cout << endl;

	return (*accepted_limiters.begin());

}

std::set<int> mono_webster_640::get_brute_force_pure_ballot(int num_seats,
	double stepsize, int a, int b, int c, int d, int e,
	int f) const {

	std::vector<solid_coalition> scs;
	scs.push_back(construct_sc("ABC", a+b+c+d+e+f));
	scs.push_back(construct_sc("AB", a+c));
	scs.push_back(construct_sc("AC", b+e));
	scs.push_back(construct_sc("BC", d+f));
	scs.push_back(construct_sc("A", a+b));
	scs.push_back(construct_sc("B", b+c));
	scs.push_back(construct_sc("C", d+e));

	return (naive_sweb(scs, num_seats, stepsize));
}

std::set<int> mono_webster_640::check_one_brute() const {
	int mxv = 1 + random() % 19;

	// Must be at least one c, or we can't very well lower, can we?
	int a = random() % mxv, b = random() % mxv, c = (random() % mxv) + 1,
		d = random() % mxv, e = random() % mxv, f = random() % mxv;

	cout << "----- Generating original council ---" << endl;

	int num_seats = 1 + random()%2;
	double stepsize = 0.005;

	std::set<int> origc = get_brute_force_pure_ballot(num_seats, stepsize, a,
			b, c, d, e, f);

	// Adjust for monotonicity.
	--c;
	++a;

	cout << "----- Generating second (A raised) council ---" << endl;

	std::set<int> newc = get_brute_force_pure_ballot(num_seats, stepsize, a,
			b, c, d, e, f);

	std::set<int> aonly; aonly.insert(0);

	cout << "All done. First council was: " << show_membership(origc);
	cout << " and the new one is " << show_membership(newc) << endl;

	bool a_in_first = includes(origc.begin(), origc.end(), aonly.begin(),
			aonly.end());
	bool a_in_second = includes(newc.begin(), newc.end(), aonly.begin(),
			aonly.end());

	if (origc != newc) {
		cout << "Something is happening." << endl;
	}

	if (a_in_first && !a_in_second) {
		cout << "Oh dear, A vanished after being raised." << endl;
		cout << "Where's my kaishakunin?" << endl;
		assert(1 != 1);
	}

	return (origc);
}

std::list<size_t> mono_webster_640::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & ballots) const {

	// Turn the input into a list of solid coalitions. (Put this code
	// elsewhere?)

	multimap<double, std::set<unsigned short> > init_coalitions =
		get_solid_coalitions(ballots, num_candidates, rand()%num_candidates);

	std::vector<solid_coalition> scs;

	for (multimap<double, std::set<unsigned short> >::const_reverse_iterator
		pos=
			init_coalitions.rbegin(); pos != init_coalitions.rend();
		++pos) {
		solid_coalition q;
		copy(pos->second.begin(), pos->second.end(), inserter(
				q.candidates, q.candidates.begin()));
		q.support = pos->first;
		scs.push_back(q);
	}

	// Turn it into the standardized format.
	sort(scs.begin(), scs.end(), greater<solid_coalition>());

	// Stepsize should probably at most be 1/numvoters. If the granularity
	// is 0.1, 0.1/numvoters, etc.. Later.

	double stepsize = min(0.01, 1.0/scs.begin()->support);
	std::set<int> council = naive_sweb(scs, council_size, stepsize);

	// But the output wants it in another format.
	std::list<size_t> toRet;
	copy(council.begin(), council.end(), inserter(toRet, toRet.begin()));

	return (toRet);
}