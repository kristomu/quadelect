#include "brute.h"
#include "../method.h"
#include "../positional/all.h"

#include <iterator>

std::vector<int> cond_brute::decode_weight_code(int wcode,
	int base) const {
	// We consider the int an integer in base 5. The integers 0,1,2,3,4
	// correspond to weights -2, -1, 0, 1, 2 for the corresponding
	// ballot ordering, which gives 5^6 (15625) different possibilities.

	// Now we use an arbitrary (odd) base. More tangibly, we could consider
	// wcode to be a number in balanced (base+1)/2-ary, and each digit gives
	// a weight to use for the linear combination that makes the brute method.
	std::vector<int> weights(6, 0);

	for (int counter = 0; counter < 6 && wcode > 0; ++counter) {
		//std::cout << wcode << "\t" << counter << "\t" << wcode%base<<std::endl;
		int this_weight = wcode % base;    // 0 .. (base-1)
		this_weight -= (base-1)/2;	   // centered on 0
		weights[counter] = this_weight;
		wcode /= base;
	}

	// Our 2x method would have weights
	// ABC ACB BAC BCA CAB CBA
	// 2   2   1   1   0   0
	// 4 + 4*5 + 3*5^2 + 3*5^3
	// = 474

	return (weights);
}

bool cond_brute::is_monotone() const {
	// Going from (i) to (ii) should never decrease the score.
	// Note that this might be too strict, e.g. there could be situations
	// where going from BAC to ABC does decrease the score, but because of
	// how the other parameters are set, it only happens when A becomes a
	// CW due to the other sums.
	//  (i)    (ii)     (num i)     (num ii)
	//  BAC     ABC         2           0
	//  CAB     ACB         4           1
	//  BCA     BAC         3           2
	//  CBA     CAB         5           4

	// So all we need to check is that the weight on i is less than or equal
	// to the weight on ii.

	// Not quite. We need to satisfy additional constraints that the others'
	// scores don't increase more than ours.

	// x0 - x2 >= x4 - x1 && x0 - x2 >= x3 - x5		(BAC->ABC)
	// x2 - x3 >= x1 - x0 && x2 - x3 >= x5 - x4		(BCA->BAC)
	// x1 - x4 >= x5 - x3 && x1 - x4 >= x2 - x0		(CAB->ACB)
	// x4 - x5 >= x3 - x2 && x4 - x5 >= x0 - x1		(CBA->CAB)

	bool oBAC =	(weights[0] - weights[2] >= weights[4] - weights[1]) &&
		(weights[0] - weights[2] >= weights[3] - weights[5]);

	bool oBCA =	(weights[2] - weights[3] >= weights[1] - weights[0]) &&
		(weights[2] - weights[3] >= weights[5] - weights[4]);

	bool oCAB =	(weights[1] - weights[4] >= weights[5] - weights[3]) &&
		(weights[1] - weights[4] >= weights[2] - weights[0]);

	bool oCBA =	(weights[4] - weights[5] >= weights[3] - weights[2]) &&
		(weights[4] - weights[5] >= weights[0] - weights[1]);

	return (oBAC && oBCA && oCAB && oCBA);

//    return (weights[0] >= weights[2] && weights[1] >= weights[4] &&
//            weights[2] >= weights[3] && weights[4] >= weights[5]);

}

bool cond_brute::passes_mat() const {
	// Passes "Half-mono-add-top": if ABCA and you add an A-first ballot and
	// the cycle is still ABCA afterwards, then A won't be hurt.

	// This is the same as mono-add-top (for complete ballots) by the
	// following argument:

	// Suppose otherwise that we have A>B>C>A with very weak B>C and want to
	// flip to A>C>B>A. Then once one ACB ballot is added, we get A>B=C>A, but
	// A can't both beat B and C and be beaten by them. So either A is CW or B
	// and C are tied as CWs. We know that A beat one of them (B) before, and
	// A couldn't have been beaten once more by B by a ballot that put
	// A first, so B and C can't both beat A, so A must be the CW.

	/*  w0 >= w4
	    w1 >= w5
	    w0 >= w3
	    w1 >= w2*/

	return (weights[0] >= weights[4] && weights[1] >= weights[5] &&
			weights[0] >= weights[3] && weights[1] >= weights[2]);
}

bool dbleq(double a, double b) {
	return (abs(a-b) < 1e-9);
}

bool cond_brute::reversal_symmetric() const {
	// Proving that the following implies reversal symmetry is an exercise
	// for the reader. Hint: Consider the transformation of the six ballot
	// categories as they're reversed and express the reversed score, which
	// must decrease when the non-reversed score increases, in terms of them.

	// w0 = w5 = 0
	// w1 = -w2
	// w3 = -w4

	return (weights[0] == 0 && weights[5] == 0 && weights[1] == -weights[2] &&
			weights[3] == -weights[4]);
}

std::string cond_brute::get_compliances() const {
	std::string compliances = "";
	if (is_monotone()) {
		compliances +="M";
	}
	if (passes_mat()) {
		compliances += "A";
	}
	if (reversal_symmetric()) {
		compliances += "R";
	}

	return (compliances);
}

std::pair<ordering, bool> cond_brute::elect_inner(const
	election_t &
	papers,
	const std::vector<bool> & hopefuls,
	int num_candidates, cache_map * cache,
	bool winner_only) const {

	// TODO: Use cache.
	assert(num_candidates == 3);

	condmat condorcet_matrix = condmat(papers, num_candidates,
			CM_PAIRWISE_OPP);

	std::vector<double> counts(6, 0); // ABC ACB BAC BCA CAB CBA

	for (election_t::const_iterator bpos = papers.begin();
		bpos != papers.end(); ++bpos) {
		// Go through the ballot in question and determine which category
		// it falls into of the complete ballots above. If neither, get
		// outta here (yeah, this is brittle as all f...). Crashes may occur
		// on unexpected input.
		ordering::const_iterator order_pos = bpos->contents.begin();
		double numvoters_this_paper = bpos->get_weight();

		std::vector<int> candorder(3, -1);
		for (int i = 0; i < 3; ++i) {
			assert(order_pos != bpos->contents.end());
			candorder[i] = (order_pos++)->get_candidate_num();
			assert(candorder[i] >= 0 && candorder[i] < 3);
		}

		// Possibilities:
		// 0>1>2    0*9+1*3+2   5   ABC
		// 0>2>1    0*9+2*3+1   7   ACB
		// 1>0>2    1*9+0*3+2   11  BAC
		// 1>2>0    1*9+2*3+0   15  BCA
		// 2>0>1    2*9+0*3+1   19  CAB
		// 2>1>0    2*9+1*3+0   21  CBA
		int index = candorder[0] * 9 + candorder[1] * 3 + candorder[2];

		switch (index) {
			case 5:
				counts[0] += numvoters_this_paper;
				break;
			case 7:
				counts[1] += numvoters_this_paper;
				break;
			case 11:
				counts[2] += numvoters_this_paper;
				break;
			case 15:
				counts[3] += numvoters_this_paper;
				break;
			case 19:
				counts[4] += numvoters_this_paper;
				break;
			case 21:
				counts[5] += numvoters_this_paper;
				break;
			default:
				std::cout << "Got a value of " << index << " not supposed to happen!" <<
					std::endl;
				std::cout << "Candorder: " << candorder[0] << " " << candorder[1] << " " <<
					candorder[2] << std::endl;
				assert(1!=1);
		}
	}

	assert(num_candidates == 3);

	ordering out;

	std::vector<double> scores_by_cand(3, -1);

	for (int counter = 0; counter < 3; ++counter) {
		double score = 0;

		// Cycle A>B>C>A
		// then counter = 0 (A)
		// sec = 1 (B),
		// third party = 2 (C)

		// SV would presumably be:
		// (B>C) / (C>A)

		//double majority = 0.5 * input.get_num_voters();

		for (int sec = 0; sec < 3; ++sec) {
			if (counter == sec) {
				continue;
			}

			int third_party = -1;
			for (int tet = 0; tet < 3 && third_party == -1; ++tet)
				if (counter != tet && sec != tet) {
					third_party = tet;
				}

			if (condorcet_matrix.get_magnitude(counter, sec) <
				condorcet_matrix.get_magnitude(sec, counter)) {
				continue;
			}

			// Now make us into A, make whoever we beat into B,
			// and whoever beats us into C.

			// We're    we beat     beaten by       reordering
			//  A       B           C               none
			//  A       C           B
			//      when it
			//      asks for
			//              we should give it
			//      ABC -> ACB  0->1
			//      ACB -> ABC  1->0
			//      BAC -> CAB  2->4
			//      BCA -> CBA  3->5
			//      CAB -> BAC  4->2
			//      CBA -> BCA  5->3
			//  B       A           C
			//      ABC -> BAC  0->2
			//      ACB -> BCA  1->3
			//      BAC -> ABC  2->0
			//      BCA -> ACB  3->1
			//      CAB -> CBA  4->5
			//      CBA -> CAB  5->4
			//  B       C           A
			//      ABC -> BCA  0->3
			//      ACB -> BAC  1->2
			//      BAC -> CBA  2->5
			//      BCA -> CAB  3->4
			//      CAB -> ABC  4->0
			//      CBA -> ACB  5->1
			//  C       A           B
			//      ABC -> CAB  0->4
			//      ACB -> CBA  1->5
			//      BAC -> ACB  2->1
			//      BCA -> ABC  3->0
			//      CAB -> BCA  4->3
			//      CBA -> BAC  5->2
			//  C       B           A
			//      ABC -> CBA  0->5
			//      ACB -> CAB  1->4
			//      BAC -> BCA  2->3
			//      BCA -> BAC  3->2
			//      CAB -> ACB  4->1
			//      CBA -> ABC  5->0

			// Make the method satisfy anonymity.
			std::vector<std::vector<int> > anonymity_permutations = {
				{0, 1, 2, 3, 4, 5},
				{1, 0, 4, 5, 2, 3},
				{2, 3, 0, 1, 5, 4},
				{3, 2, 5, 4, 0, 1},
				{4, 5, 1, 0, 3, 2},
				{5, 4, 3, 2, 1, 0}
			};

			int anon_perm_to_use = -1;

			//yuck
			// ABC
			if (counter == 0 && sec == 1 && third_party == 2) {
				anon_perm_to_use = 0;
			}
			// ACB
			if (counter == 0 && sec == 2 && third_party == 1) {
				anon_perm_to_use = 1;
			}
			// BAC
			if (counter == 1 && sec == 0 && third_party == 2) {
				anon_perm_to_use = 2;
			}
			// BCA
			if (counter == 1 && sec == 2 && third_party == 0) {
				anon_perm_to_use = 3;
			}
			// CAB
			if (counter == 2 && sec == 0 && third_party == 1) {
				anon_perm_to_use = 4;
			}
			// CBA
			if (counter == 2 && sec == 1 && third_party == 0) {
				anon_perm_to_use = 5;
			}

			assert(anon_perm_to_use >= 0);

			// Now it's just a matter of adding up all the scores.
			for (int i = 0; i < 6; ++i)
				score += counts[anonymity_permutations[anon_perm_to_use][i]] *
					weights[i];
		}

		out.insert(candscore(counter, score));
		scores_by_cand[counter] = score;

	}

	return (std::pair<ordering, bool>(out, false));
}
