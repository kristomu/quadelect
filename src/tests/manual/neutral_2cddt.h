#pragma once

/* A manual test for checking that a method passes neutrality with the election

		1: A>B
		1: B>A

	Results from DH2 show that some of the implemented methods fail even this
	simple measure of neutrality. For most deterministic methods, that's a bug,
	although some (like Ranked Pairs) do this on purpose. */

#include "../../ballots.h"
#include "../../interpreter/rank_order.h"
#include "../../singlewinner/method.h"

#include <map>
#include <string>

class two_neutrality_test {
	private:
		rank_order_int parser;
		election_t two_way_tie;

	public:
		two_neutrality_test();

		bool passes(election_method & method) const;

		std::string name() const {
			return "Two-way neutrality";
		}
};