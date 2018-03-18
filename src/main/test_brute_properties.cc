#include <values.h>
#include <assert.h>
#include <errno.h>

#include <iterator> 
#include <iostream> 
#include <fstream>
#include <list>
#include <set>

#include "../ballot_tools.h"
#include "../ballots.h"
#include "../tools.h"

#include "../generator/spatial/gaussian.h"
#include "../generator/spatial/uniform.h"
#include "../generator/impartial.h"
#include "../generator/dirichlet.h"

#include "../singlewinner/gradual_c_b.h"

#include "../singlewinner/stats/cardinal.h"
#include "../singlewinner/elimination.h"
#include "../singlewinner/positional/simple_methods.h"

#include "../tests/tests/monotonicity/mono_raise.h"

#include "../stats/stats.h"

#include "../singlewinner/brute_force/all.h"

#include "../random/random.h"

#include "strat_test.h"

int main(int argc, const char ** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " [brute_rpn_number_list.txt]"
            << endl;
        return(-1);
    }

    string name = argv[1];
    ifstream tests_in(argv[1]);
    vector<unsigned long long> tests;

    while (!tests_in.eof()) {
        unsigned long long tin;
        tests_in >> tin;
        tests.push_back(tin);
    }


    cout << "Test time!" << endl;
    cout << "Thy name is " << name << endl;

    #pragma omp parallel for
    for (size_t j = 0; j < tests.size(); ++j) {
    	cond_brute_rpn cbp(tests[j]);

        size_t attempts_per = 200000;
    	bool mono_ok = (cbp.check_monotonicity(attempts_per) == 0);
    	bool revsym_ok = (cbp.check_reversal_symmetry(attempts_per) == 0);
    	bool liia_ok = (cbp.check_liia(attempts_per) == 0);
        bool mat_ok = (cbp.check_mono_add_top(attempts_per) == 0);

        #pragma omp critical
        {
    	   cout << cbp.name() << " results: ";
    	   if (mono_ok) {
        		cout << "M";
    	   }
           if (mat_ok) {
                cout << "A";
           }
    	   if (revsym_ok) {
        		cout << "R";
    	   }
    	   if (liia_ok) {
        		cout << "L";
    	   }
            cout << endl;
        }
    }

    return(0);
}