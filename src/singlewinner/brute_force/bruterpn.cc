// RPN brute force: encode any function as k bits where k is
// any of the literals (ABC, BAC, etc)
// any of the aliases (A>B, B>C, C>A, fpA, fpB, fpC)
// any of a set type of numbers (-2, -1, 0, 1, 2)
// any of a set number of unary functions (abs, sin, cos, tanh, etc)
// any of a set number of binary functions (+ - / * min max etc)
// any of a set number of n-ary functions (cond, if, mainly)
//	https://www.cis.upenn.edu/~matuszek/LispText/lisp-cond.html

// Refuse if:
//	it doesn't include every literal (or at least some of them?)
//	the literals don't propagate (do a test for 10 or 20 random inputs
//		and see if output is the same every time)
//	the stack is empty before we get all the way through
//	there's more than one stack value after we get all the way through
//		(can be simplified)

// This has been put elsewhere. What follows is pretty much a copy of
// brute, adapted for rpn purposes.

// --- //
// Checking monotonicity by brute should be pretty easy since we know the
// transforms. Just scream bloody murder if we go from BAC to ABC (-1 +1)
// and A's score decreases. Similarly for the other variants. But this can be
// overbroad, of course. We should also check if going from BAC to ABC
// breaks the cycle in favor of A, in which case A does win by default.

// Some serious refactoring is in order here.

#include "bruterpn.h"
#include "../method.h"

#include <iterator>

// Criterion checks, internal.

#define C_ABC vote_array[0]
#define C_ACB vote_array[1]
#define C_BAC vote_array[2]
#define C_BCA vote_array[3]
#define C_CAB vote_array[4]
#define C_CBA vote_array[5]

#define idx_ABC 0
#define idx_ACB 1
#define idx_BAC 2
#define idx_BCA 3
#define idx_CAB 4
#define idx_CBA 5

bool cond_brute_rpn::is_abca(const vector<double> & vote_array) const {
    // (A>B - B>A) > 0
    // A>B = ABC + ACB + CAB
    // B>A = BAC + BCA + CBA

    if (C_ABC + C_ACB + C_CAB - (C_BAC + C_BCA + C_CBA) <= 0)
        return(false);

    // (B>C - C>B) > 0
    // B>C = BCA + BAC + ABC
    // C>B = CAB + CBA + ACB

    if (C_BCA + C_BAC + C_ABC - (C_CAB + C_CBA + C_ACB) <= 0)
        return(false);

    // (C>A - A>B) > 0
    // C>A = CAB + CBA + BCA
    // A>B = ABC + ACB + CAB

    return (C_CAB + C_CBA + C_BCA - (C_ABC + C_ACB + C_CAB) > 0);
}

bool cond_brute_rpn::get_scores(const vector<double> & vote_array,
    vector<double> & output) const {
    try {
        double ascore = cfunct.evaluate(vote_array, false);
        //
        //  B       C           A
        //      ABC -> BCA  0->3
        //      ACB -> BAC  1->2
        //      BAC -> CBA  2->5
        //      BCA -> CAB  3->4
        //      CAB -> ABC  4->0
        //      CBA -> ACB  5->1
        vector<double> transposed_to_b = {C_BCA, C_BAC, C_CBA, 
            C_CAB, C_ABC, C_ACB};

        double bscore = cfunct.evaluate(transposed_to_b, false);

        //  C       A           B
        //      ABC -> CAB  0->4
        //      ACB -> CBA  1->5
        //      BAC -> ACB  2->1
        //      BCA -> ABC  3->0
        //      CAB -> BCA  4->3
        //      CBA -> BAC  5->2

        vector<double> transposed_to_c = {C_CAB, C_CBA, C_ACB, C_ABC, 
            C_BCA, C_BAC};

        double cscore = cfunct.evaluate(transposed_to_c, false);

        output[0] = ascore;
        output[1] = bscore;
        output[2] = cscore;
    } catch (std::runtime_error & rerr) {
        return(false);
    }
    return(true);
}

// We assume the vote array is ABCA.
// Returns true if we didn't find any monotonicity problems,
// otherwise false.
bool cond_brute_rpn::check_monotonicity_single_instance(int num_attempts,
    const vector<double> & vote_array) const {

    vector<double> modified_vote_array;
    vector<double> original_scores(3), modified_scores(3);
    get_scores(vote_array, original_scores);

    // Raise A in one of a number of ways
    for (int i = 0; i < num_attempts; ++i) {
		modified_vote_array = vote_array;		// Oe lu skxawng
        // Possible raising:
        // BAC -> ABC
        // CAB -> ACB
        // BCA -> BAC
        // CBA -> CAB

        // TODO: Use proper randomness.
        int down[] = {idx_BAC, idx_CAB, idx_BCA, idx_CBA};
        int up[] =   {idx_ABC, idx_ACB, idx_BAC, idx_CAB};

        int which = random() % 4;
        int inner_tries = 10;

        for (int j = 0; j < inner_tries && modified_vote_array[down[which]] < 1; ++j) {
            which = random()%4;
        }

        // Can't check because (most likely) no candidate with more than one
	// vote can be raised.
        if (modified_vote_array[down[which]] < 1)
            continue;

        // Otherwise raise.
        modified_vote_array[down[which]] -=1;
        modified_vote_array[up[which]] += 1;

        // Get the modified scores
        get_scores(modified_vote_array, modified_scores);

        // Check that we're still ABCA. If not, A won, so that's OK
        if (!is_abca(modified_vote_array))
            continue;

        // Check that if someone ranked below A, he's not ranked above now.
        if (original_scores[1] < original_scores[0] && modified_scores[1] >= modified_scores[0]) {
			/*cout << "Type one error" << endl;
            cout << "Was: ";
            copy(vote_array.begin(), vote_array.end(), ostream_iterator<double>(cout, " "));
			cout << endl << "Scores:";
			copy(original_scores.begin(), original_scores.end(), ostream_iterator<double>(cout, " "));
            cout << endl << "Is:";
            copy(modified_vote_array.begin(), modified_vote_array.end(), ostream_iterator<double>(cout, " "));
            cout << endl << "scores:";
			copy(modified_scores.begin(), modified_scores.end(), ostream_iterator<double>(cout, " "));
			cout << endl;*/
            return (false);
        }
        if (original_scores[2] < original_scores[0] && modified_scores[2] >= modified_scores[0]) {
/*cout << "Type two error" << endl;
            cout << "Was: ";  
            copy(vote_array.begin(), vote_array.end(), ostream_iterator<double>(cout, " "));
            cout << endl << "Scores:";  
            copy(original_scores.begin(), original_scores.end(), ostream_iterator<double>(cout, " "));
            cout << endl << "Is:";  
            copy(modified_vote_array.begin(), modified_vote_array.end(), ostream_iterator<double>(cout, " "));
            cout << endl << "scores:";  
            copy(modified_scores.begin(), modified_scores.end(), ostream_iterator<double>(cout, " "));
            cout << endl;*/
            return(false);
        }
    }
    return(true);
}

// Some functional programming would be pretty nice right about now.
// I know what OOP would say, but...
int cond_brute_rpn::check_monotonicity(int num_attempts) const {

    int cur_attempt = 0;    // Don't count anything but ABCA cycles as an
                            // attempt.
    int inner_num_attempts = 10; // exempli gratia

    int failures = 0;

    for (cur_attempt = 0; cur_attempt < num_attempts && failures == 0;
        ++cur_attempt) {

        vector<double> vote_array(6, 0);

        while (!is_abca(vote_array)) {
            for (int counter = 0; counter < 6; ++counter) {
                vote_array[counter] = drand48() * 37;
            }
        }

        if (!check_monotonicity_single_instance(inner_num_attempts, 
            vote_array)) {
            ++failures;
        }
    }

    return(failures);
}

bool cond_brute_rpn::check_liia_single_instance(
    const vector<double> & vote_array) const {

    // We have ABCA
    // If A is first, then B must be second (since A>B>C>A, so without
    //      A, it'd be B>C)
    // If B is first, then C must be second.
    // If C is first, then A must be second.

    vector<double> original_scores(3);
    get_scores(vote_array, original_scores);

    // A wins
    if (vote_array[0] > vote_array[1] && vote_array[0] > vote_array[2])
        return (vote_array[1] > vote_array[2]); // B must be second
    // B wins
    if (vote_array[1] > vote_array[0] && vote_array[1] > vote_array[2])
        return(vote_array[2] > vote_array[0]); // C must be second.
    // C wins
    if (vote_array[2] > vote_array[1] && vote_array[2] > vote_array[0])
        return(vote_array[0] > vote_array[1]); // A must be second

    return(true); // inconclusive
}

int cond_brute_rpn::check_liia(int num_attempts) const {

    int cur_attempt = 0;    // Don't count anything but ABCA cycles as an
                            // attempt.

    int failures = 0;

    for (cur_attempt = 0; cur_attempt < num_attempts && failures == 0;
        ++cur_attempt) {

        vector<double> vote_array(6, 0);

        while (!is_abca(vote_array)) {
            for (int counter = 0; counter < 6; ++counter) {
                vote_array[counter] = int(drand48() * 37);
            }
        }

        if (!check_liia_single_instance(vote_array)) {
            /*cout << "Failure detected." << endl;
            copy(vote_array.begin(), vote_array.end(), ostream_iterator<double>(cout, " "));
            cout << endl;*/
            ++failures;
        }
    }

    return(failures);
}

bool cond_brute_rpn::check_revsym_single_instance(
    const vector<double> & vote_array) const {

    vector<double> original_scores(3), modified_scores(3);
    get_scores(vote_array, original_scores);

    // Reversing the ballots:
    // ABC -> CBA
    // ACB -> BCA
    // BAC -> CAB
    // BCA -> ACB
    // CAB -> BAC
    // CBA -> ABC

    vector<double> reversed_votes = {C_CBA, C_BCA, C_CAB, C_ACB, C_BAC, 
        C_ABC};

    get_scores(reversed_votes, modified_scores);

    // If A > B on both, we fail. Same for every other combination.
    // Note that we let ties pass.

    for (int cand = 0; cand < 3; ++cand) {
        for (int comparand = 0; comparand < 3; ++comparand) {
            if (comparand == cand) continue;
            if (original_scores[cand] > original_scores[comparand] &&
                modified_scores[cand] > modified_scores[comparand]) {
                return(false);
            }
        }
    }

    return(true);
}

// EW! Cut and paste code galore.
// What I wouldn't have given for FP...
// What this suggests is that everything should be extracted.
int cond_brute_rpn::check_reversal_symmetry(int num_attempts) const {

    int cur_attempt = 0;    // Don't count anything but ABCA cycles as an
                            // attempt.

    int failures = 0;

    for (cur_attempt = 0; cur_attempt < num_attempts && failures == 0;
        ++cur_attempt) {

        vector<double> vote_array(6, 0);

        while (!is_abca(vote_array)) {
            for (int counter = 0; counter < 6; ++counter) {
                vote_array[counter] = int(drand48() * 37);
            }
        }

        if (!check_revsym_single_instance(vote_array)) {
            /*cout << "Failure detected." << endl;
            copy(vote_array.begin(), vote_array.end(), ostream_iterator<double>(cout, " "));
            cout << endl;*/
            ++failures;
        }
    }

    return(failures);
}


pair<ordering, bool> cond_brute_rpn::elect_inner(
		const list<ballot_group> & papers,
		const vector<bool> & hopefuls,
		int num_candidates, cache_map & cache,
		bool winner_only) const {

	// TODO: Use cache.
    assert (num_candidates == 3);

	condmat condorcet_matrix = condmat(papers, num_candidates, 
			CM_PAIRWISE_OPP);

    vector<double> counts(6, 0); // ABC ACB BAC BCA CAB CBA

    for(list<ballot_group>::const_iterator bpos = papers.begin(); 
        bpos != papers.end(); ++bpos) {
        // Go through the ballot in question and determine which category
        // it falls into of the complete ballots above. If neither, get
        // outta here (yeah, this is brittle as all f...). Crashes may occur
        // on unexpected input.
        ordering::const_iterator order_pos = bpos->contents.begin();
        double numvoters_this_paper = bpos->weight;

        vector<int> candorder(3, -1);
        for(int i = 0; i < 3; ++i) {
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

        switch(index) {
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
                cout << "Got a value of " << index << " not supposed to happen!" << endl;
                cout << "Candorder: " << candorder[0] << " " << candorder[1] << " " << candorder[2] << endl;
                assert(1!=1);
        }
    }

	assert (num_candidates == 3);

	ordering out;

    vector<double> scores_by_cand(3, -1);

	for (int counter = 0; counter < 3; ++counter) {
                double score = 0;

                for (int sec = 0; sec < 3; ++sec) {
                        if (counter == sec) continue;

                        int third_party = -1;
                        for (int tet = 0; tet < 3 && third_party == -1; ++tet)
                            if (counter != tet && sec != tet)
                                third_party = tet;

                        if (condorcet_matrix.get_magnitude(counter, sec) <
                                condorcet_matrix.get_magnitude(sec, counter))
                            continue;

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
                        vector<vector<int> > anonymity_permutations = {
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
                        if (counter == 0 && sec == 1 && third_party == 2)
                            anon_perm_to_use = 0;
                        // ACB
                        if (counter == 0 && sec == 2 && third_party == 1)
                            anon_perm_to_use = 1;
                        // BAC
                        if (counter == 1 && sec == 0 && third_party == 2)
                            anon_perm_to_use = 2;
                        // BCA
                        if (counter == 1 && sec == 2 && third_party == 0)
                            anon_perm_to_use = 3;
                        // CAB
                        if (counter == 2 && sec == 0 && third_party == 1)
                            anon_perm_to_use = 4;
                        // CBA
                        if (counter == 2 && sec == 1 && third_party == 0)
                            anon_perm_to_use = 5;

                        assert (anon_perm_to_use >= 0);

                        // Construct the input for the function.
                        vector<double> funcinputs(6);
                        for (int i = 0; i < 6; ++i)
                            funcinputs[i] = counts[anonymity_permutations[anon_perm_to_use][i]];

                       	// Extract the score.
                       	score = cfunct.evaluate(funcinputs, false);
                       	if (!finite(score)) {
                       		if (score < 0)
                       			score = -1000;
                       		else
                       			score = 1000;
                       	}

                }

                out.insert(candscore(counter, score));
                scores_by_cand[counter] = score;

        }

	// LIIA post-hack
	// If it's a tie, we can just return right away (won't break LIIA)???
	//	(or suppose scores are A=B>C but the cycle is A>B>C, then
	//	 removing B will give A>C but the cycle says C>A, so...)
	// If it's a three-way tie, leave it alone.
	// If it's A=B>C or A>B=C, break in the direction of the cycle.
	//	If the cycle says nothing, leave it alone.
	// Otherwise (if the scores are different) preserve the winner and
	// complete by following the cycle.
	// I can't believe this is going to be monotone or reversal symmetric.

	// Breaking A=B>C in the direction of the cycle is going to be hard.
	// Suppose the cycle is ACBA. Then we can either pick A>C>B (focus on A)
	// or B>A>C (focus on B). Since B preserves C-last, that's probably the
	// one to pick.

	// So, simplified version:
	//	If there's only one candidate with max score, WLOG call him A
	// 	and the two others B and C. Place A first and break B=C in the
	//	direction of the cycle.

	//	If there are two candidates with max score, call them A and B,
	//	then produce A=B>C and break the A=B tie in the direction of
	//	the cycle.

	//	If there's a three-way tie (A=B=C)...

	// we have a problem again. If the cycle is also a tie, then OK.
	// otherwise, we can break it in three possible ways. If the cycle is
	// ABCA, it might be A>B>C, B>C>A or C>A>B. We don't know, so we should
	// either return A=B=C (which may break LIIA) or throw an exception.

	// So if it's A>B=C, just treat it as for any other (since we know who
	// the winner is): complete with either B>C or C>B depending on the
	// direction of the cycle.

	// If it's A=B>C, break A=B in the direction of the cycle.

	// If it's A=B=C and the cycle is a tie, return the true tie. Otherwise,
	// if permissive is on, return A=B=C anyway. If it's off, throw an
	// exception.

	return(pair<ordering, bool>(out, false));
}
