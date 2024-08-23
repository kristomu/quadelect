#pragma once

#include "common/ballots.h"
#include "singlewinner/positional/positional.h"

// Maybe implement: IS_DETERMINISTIC, returns either -1 (random), 0 (determ. except
// with ties) or 1 (fully determ - presumably non-neutral).

// Majoritarian council
// Vote-based reweighted council
// Quota-based reweighted with fixed elimination order (can use any method
// for either)
// Quota-based reweighted with fluid elimination order (STV, QLTD-PR, QPQ, ...)

// How are we going to handle ties here? A list of lists could do it, with
// every possible tie, but in the case of a complete tie, that returns
// exponential numbers of lists. Perhaps a "was_tie" boolean? Hm.

class multiwinner_method {

	public:
		virtual std::list<int> get_council(int council_size,
			int num_candidates, const election_t & ballots) const = 0;

		virtual std::string name() const = 0;

		// Hack?
		virtual bool polytime() const {
			return true;
		}

};

class majoritarian_council : public multiwinner_method {
	private:
		election_method * base;

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("Maj[" + base->name() + "]");
		}

		majoritarian_council(election_method * base_in) {
			base = base_in;
		}

};

class random_council : public multiwinner_method {
	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("Random council");
		}

};

// --- //

// Multiplicative voter-based reweighting.
// 	WeightNew = WeightOld * A / (B + curscore / maxscore)
// 	C / (/*C +*/ inner_denom)
class mult_ballot_reweighting : public multiwinner_method {

	private:
		positional * base; // But not Bucklin or QLTD
		double A, B;

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		std::string name() const {
			return ("ReweightM[" +base->name() + "]");
		}

		mult_ballot_reweighting(positional * base_in) {
			base = base_in; A = 0.5; B = 1;
		}
		mult_ballot_reweighting(positional * base_in, double A_in,
			double B_in) {
			base = base_in; A = A_in;
			B = B_in;
		}

};

// Additive voter-base reweighting.
// Say the original weight is origwt. Then at some round,
// 	weight = origwt * C / (C + Q)
// 	where
// 		C = M (D'Hondt) or M/2 (Sainte-Lague)
// 		M = maximum attainable score of all the rounds
// 		Q = sum of score given to elected candidates
// There are two ways this can be treated. Either M is the same for all rounds
// for positional systems, or it's the maximum given the number of unelected
// candidates left. The latter seems to give better scores. For Range, M is
// fixed and so the two approaches are equal.
class addt_ballot_reweighting : public multiwinner_method {

	private:
		positional * base; // But not QLTD or Bucklin. But Range.

	public:
		std::list<int> get_council(int council_size, int num_candidates,
			const election_t & ballots) const;

		addt_ballot_reweighting(positional * base_in) {
			base = base_in;
		}
		std::string name() const {
			return ("ReweightA[" +base->name() + "]");
		}
};