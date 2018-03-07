#ifndef _VOTE_CW_BRUTE_RPN
#define _VOTE_CW_BRUTE_RPN

#include "../../pairwise/matrix.h"
#include "rpn/custom_func.h"
#include "../pairwise/method.h"
#include "../method.h"

#include <iterator>
#include <iostream>
#include <assert.h>

using namespace std;

class cond_brute_rpn : public election_method {
	private:
		custom_function cfunct;		// To do later: inheritance and all that jazz
		const string ext = "RPN";

		unsigned long long id;
		list<double> stack;

		bool is_abca(const vector<double> & vote_array) const;
		bool get_scores(const vector<double> & vote_array,
    		vector<double> & output) const;

		// TODO later: refactor these out into an overload of the relevant
		// criterion checkers. Need to implement criterion checkers first.
		bool check_monotonicity_single_instance(int num_attempts,
    		const vector<double> & vote_array) const;
		bool check_liia_single_instance(
			const vector<double> & vote_array) const;
		bool check_revsym_single_instance(
    		const vector<double> & vote_array) const;

	public:
		pair<ordering, bool> elect_inner(
				const list<ballot_group> & papers,
				const vector<bool> & hopefuls,
				int num_candidates, cache_map * cache,
				bool winner_only) const;

		string name() const {
			return ("Brute " + ext + "(" + lltos(id) + "," + 
				get_printable_representation() + ")");
		}

		int check_monotonicity(int num_attempts) const;
		int check_liia(int num_attempts) const;
		int check_reversal_symmetry(int num_attempts) const;

		void set_funct_code(unsigned long long funct_code_in) {
			id = funct_code_in;
			cfunct.set_id(id);
		}

		cond_brute_rpn(unsigned long long funct_code_in) {
			set_funct_code(funct_code_in);
		}

		string get_printable_representation() const {
			vector<string> functs = cfunct.get_atom_printout();
			string out;
			for (size_t i = 0; i < functs.size(); ++i) {
				out = out + functs[i] + " ";
			}
			return(out);
		}
};

#endif