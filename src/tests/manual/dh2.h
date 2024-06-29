#pragma once

/* A manual test for a DH2-type relative burial resistance criterion.
   This criterion says that with the following elections:

	honest:
		50000: A>B>C
		50000: B>A>C
		    1: C>A>B
		    1: C>B>A

	A_buries:
		50000: A>C>B  (B buried)
		50000: B>A>C
		    1: C>A>B
		    1: C>B>A

	B_buries:
		50000: A>B>C
		50000: B>C>A
		    1: C>A>B
		    1: C>B>A

	both_bury: (mutual assured destruction?)
		50000: A>C>B
		50000: B>C>A
		    1: C>A>B
		    1: C>B>A

	it must not be the case that both one of the voting factions
	can break an initial tie (or probabilistic election) by burying,
	and that if both factions do so, then C is elected.

	Strictly speaking:
		if honest doesn't elect both A and B, then pass
		if A_buries doesn't turn B from a winner to a loser, then pass
		if B_buries doesn't turn A from a winner to a loser, then pass
		if both_bury doesn't elect C, then pass
		otherwise fail.

	Passing this criterion implies passing Monroe's NIA in most cases.
	(TODO: Formally explain the preconditions.) We would also want the
	C-first voters' weight to approach zero (or the A and B voters'
	to approach infinity) to keep the criterion from depending on
	arbitrary constants.

	Note that the reverse direction isn't true. Range passes NIA but
	fails this (with Borda scores). This because the factions have
	safer options that will secure their candidate's victory, yet won't
	blow up when both do it at once. */

#include "../../ballots.h"
#include "../../interpreter/rank_order.h"
#include "../../singlewinner/method.h"
#include "../../tools/ballot_tools.h"

#include <map>
#include <string>

class dh2_test {
	private:
		rank_order_int parser;
		names_and_election honest, A_buries, B_buries, both_bury;

	public:
		dh2_test();

		bool passes(election_method & method, bool verbose) const;
		bool passes(election_method & method) const {
			return passes(method, false);
		}
};