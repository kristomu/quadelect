#include <assert.h>
#include <errno.h>

#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>

#include "tools/ballot_tools.h"
#include "common/ballots.h"
#include "tools/tools.h"

#include "generator/spatial/gaussian.h"
#include "generator/spatial/uniform.h"
#include "generator/impartial.h"
#include "generator/dirichlet.h"

#include "singlewinner/gradual_c_b.h"

#include "singlewinner/stats/cardinal.h"
#include "singlewinner/elimination.h"
#include "singlewinner/positional/simple_methods.h"

#include "tests/tests/monotonicity/mono_raise.h"

#include "stats/stats.h"

#include "singlewinner/brute_force/all.h"

#include "random/random.h"

#include "tests/strategies.h"

int main(int argc, const char ** argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [brute_rpn_number_list.txt]"
			<< std::endl;
		return (-1);
	}

	string name = argv[1];
	ifstream tests_in(argv[1]);
	std::vector<unsigned long long> tests;

	while (!tests_in.eof()) {
		unsigned long long tin;
		tests_in >> tin;
		tests.push_back(tin);
	}


	std::cout << "Test time!" << std::endl;
	std::cout << "Thy name is " << name << std::endl;

	size_t j;
	const int stepsize = 1000;

	for (size_t i = 0; j < tests.size(); i += stepsize) {
		#pragma omp parallel for
		for (size_t k = 0; k < stepsize; ++k) {
			j = i+k;
			if (j >= tests.size()) {
				continue;
			}

			cond_brute_rpn cbp(tests[j]);

			size_t attempts_per = 10000;
			bool mono_ok = (cbp.check_monotonicity(attempts_per) == 0);
			bool revsym_ok = (cbp.check_reversal_symmetry(attempts_per) == 0);
			bool liia_ok = (cbp.check_liia(attempts_per) == 0);
			bool mat_ok = (cbp.check_mono_add_top(attempts_per) == 0);
			bool wpd_ok = (cbp.check_weak_positional_dominance(attempts_per) == 0);
			bool dmtbr_ok = (cbp.check_dmtbr(attempts_per) == 0);

			#pragma omp critical
			{
				std::cout << cbp.name() << " results: ";
				if (mono_ok) {
					std::cout << "M";
				}
				if (mat_ok) {
					std::cout << "A";
				}
				if (revsym_ok) {
					std::cout << "R";
				}
				if (liia_ok) {
					std::cout << "L";
				}
				if (wpd_ok) {
					std::cout << "P";
				}
				if (dmtbr_ok) {
					std::cout << "D";
				}
				std::cout << std::endl;
			}
		}
	}

	return (0);
}