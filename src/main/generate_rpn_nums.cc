#include <iostream>
#include <iterator>
#include <vector>
#include <list>
#include <map>

#include <math.h>
#include <stdlib.h>

#include "../singlewinner/brute_force/rpn/custom_func.h"
#include "../singlewinner/brute_force/bruterpn.h"

using namespace std;

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

void get_digits_msb(list<int> & output, unsigned long long number,
    int radix) {

    if (number == 0)
        return;

    output.push_front(number%radix);
    get_digits_msb(output, number/radix, radix);
}

unsigned long long digits_to_number(const list<int> & input, int radix) {

    unsigned long long num_out = 0;

    for (list<int>::const_iterator pos = input.begin(); 
        pos != input.end(); ++pos) {

        num_out = num_out * radix + *pos;
    }

    return(num_out);
}

// takes the input number as an index into a list of all possible methods
// that only have the atoms in the specified permitted vector, and
// transform it into a bruterpn number that obeys that constraint.
// (Explain better later)
unsigned long long transform_rpn_number_only_these(unsigned long long idx,
    const vector<custom_funct_atom> & permitted) {

    list<int> digits;

    get_digits_msb(digits, idx, permitted.size());

    for (list<int>::iterator pos = digits.begin(); pos != digits.end();
        ++pos) {

        *pos = (int)permitted[*pos];
    }

    return(digits_to_number(digits, (int)TOTAL_NUM_ATOMS));

}


int main() {
	vector<double> initvalues={1, 2, 3, 4, 5, 6};
	map<vector<double>, unsigned long long> already_seen;
	map<vector<bool>, unsigned long long> already_seen_ordinal;

	int numtests = 8;
	int numtests_ordinal = 256;

	vector<vector<double> > test_vectors = create_test_vectors(numtests);
	vector<vector<double> > test_vectors_ordinal = 
		create_test_vectors(numtests_ordinal);

	custom_function cf(8469866259);
    cond_brute_rpn cbp(8469866259);

	unsigned long long funct_id;
	unsigned long long start_at = 0;

    vector<custom_funct_atom> permitted = {VAL_IN_ABC, VAL_IN_ACB,
        VAL_IN_BAC, VAL_IN_BCA, VAL_IN_CAB, VAL_IN_CBA, VAL_IN_FPA,
        VAL_IN_FPB, VAL_IN_FPC, VAL_IN_AbB, VAL_IN_AbC, VAL_IN_BbA, 
        VAL_IN_BbC, VAL_IN_CbB, VAL_IN_CbA,
        VAL_IN_ALL, VAL_ZERO, VAL_ONE, VAL_TWO, UNARY_FUNC_NEG,
        BINARY_FUNC_PLUS, BINARY_FUNC_MINUS, BINARY_FUNC_MAX,
        BINARY_FUNC_MIN};

    bool use_only_permitted_tokens = false;
    bool require_monotone = false;

	for (unsigned long long i = start_at; i >= 0; ++i) {
		if ((i & 8388607) == 0)
			cerr << i << "  " << round(100.0*log(i+1-start_at)/log(10))/
				100.0 << "     \r" << flush;

		if (use_only_permitted_tokens) {
			funct_id = transform_rpn_number_only_these(i, permitted);
		} else {
        	funct_id = i;
        }

		cf.set_id(funct_id);
        cbp.set_funct_code(funct_id);

        /*cout << "Function " << funct_id << ": ";
        vector<string> printout = cf.get_atom_printout();
            copy(printout.begin(), printout.end(),
                ostream_iterator<string>(cout, " "));*/
		if (cf.update_suitability(test_vectors, already_seen)) {
			if (require_monotone && cbp.check_monotonicity(40000) > 0) {
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