// Cycle-cutting, based on my "Ranked Triples" idea:

// http://lists.electorama.com/pipermail/election-methods-electorama.com/2024-October/006775.html
// "A cloneproof fpA-fpC method?"

// First determine what three-cycles exist, making use of the property
// that any larger cycle can be decomposed into one or more three-candidate
// cycles.

// Then break these cycles, one by one, at the edge pointing inwards
// to the fpA-fpC winner when all candidates but the ones in the cycle
// have been eliminated.

// Once this is done, use a Condorcet method on the altered matrix. It
// shouldn't matter which, I think, though I'd have to check just to be sure.

// Currently this is non-neutral: any pairwise ties are altered to prefer
// the candidate with the lesser number. Doing so makes corner cases easier
// to handle, and should make the method closer to LIIA[1].

// The method is currently O(n^3) summable, passes DMTCBR, is cloneproof,
// and has pretty good strategy resistance that is more robust to adding
// candidates than, say, fpA - max fpC is.

// However, it's not monotone; I have to check why. In addition, it's
// far from optimized and so runs pretty slowly. It doesn't handle
// equal-rank or truncation yet, either, and the way one has to sort
// candidates to look up stuff in subelect_count_t is pretty ugly.

// [1] http://lists.electorama.com/pipermail/election-methods-electorama.com/2025-January/006813.html

#pragma once

#include "../method.h"
#include "../meta/comma.h"
#include "../sets/max_elements/smith.h"

#include "pairwise/matrix.h"

typedef std::map<std::vector<size_t>, std::vector<double> >
subelect_count_t;

// Helper structure; this contains the cycle members and
// their fpA-fpC scores.
class cycle_fpa_fpc {
	public:
		std::vector<size_t> cycle; // in cycle order, e.g. {0, 1, 2} is A>B>C>A
		double fpA_fpC_score;

		cycle_fpa_fpc(const std::vector<size_t> & cycle_in,
			const subelect_count_t & subelect_count,
			bool verbose);

		// We need an empty constructor for std::vector to work
		// properly. Don't use this.
		cycle_fpa_fpc();

		bool operator<(const cycle_fpa_fpc & other) const {
			return fpA_fpC_score < other.fpA_fpC_score;
		}

		bool operator>(const cycle_fpa_fpc & other) const {
			return fpA_fpC_score > other.fpA_fpC_score;
		}
};

class cycle_cutting : public election_method {
	private:
		bool is_cycle(const condmat & matrix,
			const std::vector<size_t> & c) const;

		// For trying to figure out what's going on with
		// different matrix types and why it makes such a
		// difference to strategy resistance.
		pairwise_type initial_type;

		// Original is the "buggy" version that seems to have
		// better strategy resistance.
		bool original;

		smith_set smith;

	public:
		std::pair<ordering, bool> elect_inner(
			const election_t & papers,
			const std::vector<bool> & hopefuls,
			int num_candidates, cache_map * cache,
			bool winner_only) const;

		std::string name() const {
			if (original) {
				return "Cycle-cutting(" + initial_type.explain() + ", original)";
			}
			return "Cycle-cutting(" + initial_type.explain() + ")";
		}

		cycle_cutting(pairwise_type initial_type_in, bool is_original) {
			initial_type = initial_type_in;
			original = is_original;
		}

		cycle_cutting() {
			initial_type = CM_PAIRWISE_OPP;
			original = false;
		}

		//void test();
};

// Would be nice if I could make a test here, but I don't want
// to pull the interpreter mode into this class just to do it...
// I probably need some kind of test program.

//cycle_cutting::test() {
/*
	25: A > B > C
	14: A > C > B
	19: B > A > C
	24: B > C > A
	27: C > A > B
	16: C > B > A

	This is an A>B>C>A cycle with A having 39 first preferences,
	B and C both having 43 first prefs. So the fpA-fpB scores are:
		A: 39-43 = -4
		B: 43-39 = 4
		C: 43-43 = 0
	so B should win.

Another:
	5: A > B > C
	1: A > C > B
	2: B > A > C
	6: B > C > A
	7: C > A > B
	3: C > B > A

	A>B>C>A cycle, first prefs: 6, 8, 10
		fpA-fpC scores: -4, 2, 2

		So the winner should be either B or C, and then follow
		the cycle (B>C>A or C>A>B).

And a tie example where reducing to fpA-fpC conflicts with being close
to LIIA:
	23: A > B > C
	27: A > C > B
	20: B > A > C
	25: B > C > A
	13: C > A > B
	18: C > B > A

	A=B and both defeat C pairwise. Arguably the right decision
	here is A=B>C due to arguments around passing LIIA at least some
	of the time, e.g. if we break the tie otherwise, then
	eliminating C leaves the method with a perfect pairwise tie
	which can't be resolved either way.

	This example isn't actually a cycle since C doesn't beat either A or B.
	But if we had {A=B, B>C, C>A}, then we might need to break the C>A
	victory (which is not yet implemented).

A possible LIIA failure:
	4: A > B > C
	5: A > C > B
	1: B > A > C
	4: B > C > A
	1: C > A > B
	5: C > B > A

	This requires a tiebreaker to be imposed ahead of time or it will
	necessarily fail LIIA (I think).

A monotonicity failure:
	1: A > C > D > B
	1: A > D > B > C
	2: B > C > A > D
	1: C > B > D > A
	1: C > D > B > A
	1: D > A > B > C
	1: D > B > A > C

	C wins. Then raise C:

	1: A > C > D > B
	1: A > D > B > C
	2: C > B > A > D		<-- here
	1: C > B > D > A
	1: C > D > B > A
	1: D > A > B > C
	1: D > B > A > C

	and A wins.

	This happens because raising C leads C to win against B, which
	produces a cycle, which then is broken in a way that doesn't favor C,
	because it removes a piece of a later cycle that would have given C
	the victory when broken according to fpA-fpC.

	I can't quite see how to keep that from happening, apart from
	something very ugly where we check if it's to anybody's benefit
	if they were to lose this cycle instead of winning... or by going
	the other way and whenever A>B, B>C, "pretend" there's a C>A and
	see if that would benefit A.

}*/