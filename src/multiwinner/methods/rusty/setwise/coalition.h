#pragma once

#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

#include <unordered_map>

#include "common/ballots.h"

using namespace std;

class solid_coalition {
	public:
		set<int> candidates;
		double support;
		bool less(const solid_coalition & rhs) const {
			if (support != rhs.support) {
				return (support < rhs.support);
			} else    return (candidates.size() < rhs.candidates.
						size());
		}
		bool equal(const solid_coalition & rhs) const {
			return (support == rhs.support && candidates.size() ==
					rhs.candidates.size());
		}
		bool operator< (const solid_coalition & rhs) const {
			return (less(rhs));
		}

		bool operator> (const solid_coalition & rhs) const {
			return (!less(rhs) && !equal(rhs));
		}
};

class sorted_reference {
	public:
		const solid_coalition * check;
		double overriding_support;

		bool less(const sorted_reference & rhs) const {
			if (overriding_support != rhs.overriding_support)
				return (overriding_support <
						rhs.overriding_support);
			else	return (check->candidates.size() <
						rhs.check->candidates.size());
		}

		bool equal(const sorted_reference & rhs) const {
			return (overriding_support == rhs.overriding_support &&
					check->candidates.size() ==
					rhs.check->candidates.size());
		}

		bool operator<(const sorted_reference & rhs) const {
			return (less(rhs));
		}

		bool operator>(const sorted_reference & rhs) const {
			return (!less(rhs) && !equal(rhs));
		}

		sorted_reference(const solid_coalition * check_in) {
			check = check_in;
		}
};

vector<solid_coalition> get_acquiescing_coalitions(
	const list<ballot_group> & input, int num_candidates);