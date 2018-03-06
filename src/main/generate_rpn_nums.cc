#include <iostream>
#include <iterator>
#include <vector>
#include <list>
#include <map>

#include <math.h>
#include <stdlib.h>

#include "../singlewinner/brute_force/rpn/custom_func.h"

using namespace std;

/* Avert your eyes, ye who enter... Cut and paste mania starts here */

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

bool is_abca(const vector<double> & vote_array) {
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

bool get_scores(const vector<double> & vote_array,
    vector<double> & output, const custom_function & cfunct) {
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
bool check_monotonicity_single_instance(int num_attempts,
    const vector<double> & vote_array, const custom_function & cfunct) {

    vector<double> modified_vote_array;
    vector<double> original_scores(3), modified_scores(3);
    get_scores(vote_array, original_scores, cfunct);

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

        // Can't check because (most likely) every raisable is < 1
        if (modified_vote_array[down[which]] < 1)
            continue;

        // Otherwise raise.
        modified_vote_array[down[which]] -=1;
        modified_vote_array[up[which]] += 1;

        // Get the modified scores
        get_scores(modified_vote_array, modified_scores, cfunct);

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
int check_monotonicity(int num_attempts, const custom_function & cfunct) {

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
            vote_array, cfunct)) {
            ++failures;
        }
    }

    return(failures);
}


// You may reopen your eyes here.

vector<vector<double> > create_test_vectors(int how_many) {
	vector<vector<double> > test_vectors;

	for (int test = 0; test < how_many; ++test) {
		vector<double> this_test(6, 0);
		for (int tidx = 0; tidx < 6; ++tidx) {
			this_test[tidx] = drand48()*8191;
		}
		test_vectors.push_back(this_test);
	}

	return (test_vectors);
}

int main() {
	vector<double> initvalues={1, 2, 3, 4, 5, 6};
	map<vector<double>, unsigned long long> already_seen;
	map<vector<bool>, unsigned long long> already_seen_ordinal;

	int numtests = 8;
	int numtests_ordinal = 128;

	vector<vector<double> > test_vectors = create_test_vectors(numtests);
	vector<vector<double> > test_vectors_ordinal = 
		create_test_vectors(numtests_ordinal);

	unsigned long long huh = 12111803; // why isn't this one recorded?

	custom_function cf(8469866259);
	vector<string> printout = cf.get_atom_printout();
			/*copy(printout.begin(), printout.end(),
				ostream_iterator<string>(cout, " "));*/
	//return(0);

	unsigned long long funct_id;
	unsigned long long start_at = 0; // 56751697515LL;

	for (unsigned long long i = start_at; i >= 0; ++i) {
		if ((i & 8388607) == 0)
			cerr << i << "  " << round(100.0*log(i+1-start_at)/log(10))/100.0 << "     \r" << flush;
		funct_id = i;
		cf.set_id(funct_id);
		//cout << "Function " << i << ": ";
		if (cf.update_suitability(test_vectors, already_seen)) {
			if (!check_monotonicity(40000, cf)) {
				//cout << "Not monotone" << endl;
				continue;
			}
			if (!cf.update_ordinal_suitability(test_vectors_ordinal, 
				already_seen_ordinal)) {
				//cout << "(fail ordinal)" << endl;
				continue;
			}
			cout << "Function " << funct_id << ": ";
			cout << "OK, " << cf.evaluate(initvalues, false) << "\t";
			vector<string> printout = cf.get_atom_printout();
			copy(printout.begin(), printout.end(),
				ostream_iterator<string>(cout, " "));
			cout << endl;
		} else {
			//cout << "(fail cardinal)" << endl;
		}
	}

	return(0);
}
