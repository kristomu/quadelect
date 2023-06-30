// Young's method. See young.h for more information!

// Possible later thing to do: use the virtual equation stuff in
// linear_model/ to simplify the construction of the GLPK model.

#include "../pairwise/matrix.h"
#include "method.h"

#include <iterator>
#include <iostream>

#include <glpk.h>
#include <assert.h>

#include "young.h"


std::pair<double, double> young::get_young_score(const
	std::list<ballot_group> &
	papers,
	size_t candidate, size_t num_candidates, size_t num_ballots,
	const std::vector<bool> & hopeful, bool relaxed,
	bool symmetric_completion, bool debug) const {

	std::pair<double, double> score(-1, -1);

	assert(candidate < num_candidates);
	if (!hopeful[candidate]) {
		return (score);
	}

	// Yup, you guessed it, it's return of GLPK.

	// Create the problem we want to solve. (Maybe move this off-function
	// later?)
	glp_prob * ip;
	ip = glp_create_prob();
	assert(ip != NULL);
	glp_set_prob_name(ip, std::string("Young score for " +
			itos(candidate)).c_str());
	glp_set_obj_dir(ip, GLP_MAX); // Maximum score.

	// Row = alias variables, e.g. p = sum over i x[i] * e[i][c*][1],
	// then later, p > 0. There are (num_hopefuls - 1) of these, but we
	// just fix them to num_candidates-1 and have everybody beat the not-
	// hopefuls - for simplicity's sake.

	// Columns = terms (e[0][c*][1], e[1][c*][1], e[2][c*][1]...).
	//	We'll need num_ballots of these.

	// c*'s Young score:
	//      max sum over values x[i] so that
	//              for all candidates a except for c*,
	//                      sum over i
	//                               x[i] * e[i][c*][a]
	//                        >= 1
	// for all i, x[i] is integer, 0 >= x[i] >= wt[i], where wt[i] is
	// the weight of the ith ballot,

	glp_add_rows(ip, num_candidates - 1);
	glp_add_cols(ip, num_ballots);

	// We also need ia, ja, and ar parameters for the matrix itself.
	// if ia[x] is 1, ja[x] is 2, and ar[x] is 3, that means that
	// the first (1.) alias parameter contains the second (2.) optimization
	// variable, times 3.

	// ia[x] will contain the candidate number since the first row deals
	// with c*'s victory over the first candidate, the second row deals with
	// c*'s victory over the second candidate, and so on. So x's maximum
	// value is (num_candidates - 1) * (num_ballots).

	// ja increases with ballot number, and ar is 1, 0, -1, depending
	// on whether the voter ranked c* above, equal, or below the other
	// candidate in question.

	// Since the ballots aren't random access, the easiest way to handle
	// this is to keep ja fixed and then set ia to the candidate number
	// (plus one, since glpk starts at 1), and ar to the relevant victory
	// number.

	int entries = (num_candidates - 1) * (num_ballots);
	int * ia = new int[entries + 1];
	int * ja = new int[entries + 1];
	double * ar = new double[entries + 1];
	int running_ballot_count = 1, running_count = 1;
	size_t counter;

	std::vector<double> vs_others(num_candidates);

	std::string name;

	double tie_val = 0;

	// Setting tie_val to 0.5 may make it break Condorcet when there are
	// many voters. However, having this value > 0 (e.g. 0.01) seems to
	// improve BR. More investigation is needed.
	// Setting it to (num_candidates * (num_candidates - 1)) seems to work.
	if (symmetric_completion)
		//tie_val = 0.5;
	{
		tie_val = 1/(double)(num_candidates * (num_candidates-1));
	}

	for (std::list<ballot_group>::const_iterator pos = papers.begin(); pos !=
		papers.end(); ++pos) {
		// If not relaxed, we can't handle non-integer ballot sizes.
		assert(relaxed || pos->get_weight() == round(pos->get_weight()));

		// Determine the data we need for ar. This is sort of a copy
		// of the code in matrix.cc, but to do it through a Condorcet
		// matrix would take a whole lot longer.

		ordering::const_iterator ourcand = pos->contents.end(), opos;

		// First find the current candidate.
		for (opos = pos->contents.begin(); opos != pos->contents.end()
			&& ourcand == pos->contents.end(); ++opos)
			if ((size_t)opos->get_candidate_num() == candidate) {
				ourcand = opos;
			}

		if (ourcand == pos->contents.end()) {
			// If the candidate wasn't found, that means the voter
			// truncated and thus everybody else who is ranked is
			// above him, all other hopefuls equal, not-hopefuls
			// below.

			if (debug) {
				std::cout << "Didn't find that candidate." << std::endl;
			}

			for (counter = 0; counter < hopeful.size(); ++counter)
				if (hopeful[counter] || counter == candidate) {
					vs_others[counter] = tie_val;
				} else	{
					vs_others[counter] = 1;
				}

			for (opos = pos->contents.begin(); opos !=
				pos->contents.end(); ++opos) {
				vs_others[opos->get_candidate_num()] = -1;
			}
		} else {
			// If the candidate was found, check for all others
			// whether c* ranks above them. If c* does, 1; if he
			// doesn't, either 0 or -1. Truncated and not-hopeful
			// candidates aren't checked and so get an automatic 1.

			fill(vs_others.begin(), vs_others.end(), 1);

			if (debug) {
				std::cout << "Found that candidate." << std::endl;
			}

			for (opos = pos->contents.begin(); opos !=
				pos->contents.end(); ++opos) {
				if (!hopeful[opos->get_candidate_num()]) {
					continue;
				}

				if (opos->get_score() > ourcand->get_score())
					vs_others[opos->get_candidate_num()]
						= -1;
				if (opos->get_score() == ourcand->get_score())
					vs_others[opos->get_candidate_num()]
						= tie_val;
			}
		}

		if (debug) {
			std::cout << "Our candidate is number " << candidate << std::endl;
			std::cout << "The versus array is: ";
			copy(vs_others.begin(), vs_others.end(),
				std::ostream_iterator<int>(std::cout, " "));
			std::cout << std::endl;
		}

		for (counter = 0; counter < vs_others.size(); ++counter) {
			if (counter == candidate) {
				continue;
			}

			if (counter < candidate) {
				ia[running_count] = 1 + counter;
			} else	{
				ia[running_count] = counter;
			}

			ja[running_count] = running_ballot_count;
			ar[running_count] = vs_others[counter];
			++running_count;
		}

		// Set the column constraint for this voter. The x parameter
		// can't be above the ballot's weight, nor can it be below
		// zero.

		glp_set_col_bnds(ip, running_ballot_count, GLP_DB, 0,
			pos->get_weight());

		if (debug) {
			name = "x[" + itos(running_ballot_count - 1) + "]";
			glp_set_col_name(ip, running_ballot_count,
				name.c_str());
		}

		if (!relaxed) {
			glp_set_col_kind(ip, running_ballot_count, GLP_IV);
		}

		++running_ballot_count;
	}

	assert(running_count <= entries + 1);
	glp_load_matrix(ip, running_count - 1, ia, ja, ar);

	// Now set row constraints. These must all be above zero, i.e. the
	// candidate must win pairwise against each of them.

	// Unfortunately, GLPK only supports >=. This is not a problem in
	// the integer case since we can just set >= 1, but in the relaxed
	// case, it may cause a slight discontinuity. TODO: Experiment with
	// this. Any value between 0 and 1 is consistent since the IP version
	// can only satisfy it by going to 1 or above.
	// Perhaps std::min(1, mindelta), where mindelta is a->weight - b->weight
	// with a and b chosen to minimize the value while still being above
	// zero? Eh, it should be independent of ballot size, I think, and then
	// 1 is as good a value as any. Just remember this when you use ballot
	// weights < 1.

	for (counter = 0; counter < num_candidates - 1; ++counter) {
		size_t realcand = counter;
		if (realcand >= candidate) {
			++realcand;
		}

		if (debug) {
			name = "vs_cand_" + itos(realcand);
			glp_set_row_name(ip, counter + 1, name.c_str());
		}

		glp_set_row_bnds(ip, counter + 1, GLP_LO, 1, 1);
	}

	// Finally, set the objective coefficients. They will all be one,
	// since no voter is more important than another.

	for (counter = 0; counter < num_ballots; ++counter) {
		glp_set_obj_coef(ip, counter + 1, 1);
	}

	// All done. Now let's set a few parameters involving verbosity and
	// then solve. We first solve the linear programming relaxation since
	// the presolving done there will greatly speed up the problem. If
	// relaxed is on or the solution is integer, we're done, otherwise
	// continue to the integer programming solver. The actual score
	// is simply the objective.

	glp_smcp params;
	glp_init_smcp(&params);

	if (debug) {
		params.msg_lev = GLP_MSG_ALL;
	} else	{
		params.msg_lev = GLP_MSG_OFF;
	}
	params.presolve = GLP_ON;

	/*glp_write_lp(ip, NULL, "output.txt");
	assert (1 != 1);*/

	// Solve the relaxation. If it doesn't succeed and it's not an
	// infeasibility problem (see later) , clean up and return -1
	// for error.
	int simplex_return = glp_simplex(ip, &params);
	bool no_soln = (simplex_return == GLP_ENOPFS);

	if (simplex_return != 0 && simplex_return != GLP_ENOPFS) {
		if (debug) {
			std::cout << "First error, val = " << simplex_return << std::endl;
		}

		delete[] ar;
		delete[] ia;
		delete[] ja;
		glp_delete_prob(ip);
		return (score);
	}

	// Check whether there are any feasible solutions to the problem at all.
	// If not, there's no way to make the candidate the CW. This can happen,
	// for instance, if he's rated last on every ballot. If there's no way
	// to make the candidate the CW, his score is 0.
	if (no_soln || glp_get_status(ip) == GLP_NOFEAS) {
		if (debug)
			std::cout << "No possible way to make this candidate a CW."
				<< std::endl;
		score = std::pair<double, double>(0, 0);
	} else {
		// So there *is* a potential way to make him the CW.

		// If we want the relaxation or we're all integer, return the
		// objective straight ahead.

		score.first = glp_get_obj_val(ip);

		bool relaxed_okay = true;

		for (counter = 0; counter < num_ballots && relaxed_okay;
			++counter) {
			double retval = glp_get_col_prim(ip, counter+1);
			relaxed_okay = (retval == (int)retval);
		}

		if (relaxed_okay) {
			score.second = glp_get_obj_val(ip);
		}

		// If the linear solution isn't integer-perfect and we
		// want an integer solution, provide it.
		if (!relaxed_okay && !relaxed) {

			// Looks like we need to use the IP solver.

			glp_iocp io_param;
			glp_init_iocp(&io_param);

			if (debug) {
				io_param.msg_lev = GLP_MSG_ALL;
			} else	{
				io_param.msg_lev = GLP_MSG_OFF;
			}

			// Make use of the advanced cut options, as they
			// significantly speed up the search.
			io_param.bt_tech = GLP_BT_BPH;
			io_param.gmi_cuts = GLP_ON;
			io_param.mir_cuts = GLP_ON;

			// I could have implemented a "non-exact" version that
			// finds out the maximum error we can have while still
			// getting the same ranking, and then have set that as
			// mip_gap, but my version of GLPK seems to have a bug
			// where it immediately times out if mip_gap != 0.

			if (glp_intopt(ip, &io_param) != 0) {
				score.second = -1;
			} else {
				score.second = glp_get_obj_val(ip);
			}
		}
	}

	// Clean up and return.

	delete[] ia;
	delete[] ja;
	delete[] ar;
	glp_delete_prob(ip);

	return (score);
}

std::string young::determine_name() const {
	std::string retval = "Young(";

	if (is_sym_comp) {
		retval += "sym-completion,";
	} else {
		retval += "wv,";
	}

	if (is_relaxed) {
		retval += "linear)";
	} else {
		retval += "integer)";
	}

	return (retval);
}

std::pair<ordering, bool> young::elect_inner(const std::list<ballot_group>
	& papers,
	const std::vector<bool> & hopefuls, int num_candidates,
	cache_map * /*cache*/, bool /*winner_only*/) const {

	// Rather simple.

	ordering toRet;

	int num_ballots = papers.size(); // O(n)

	for (int counter = 0; counter < num_candidates; ++counter) {
		if (!hopefuls[counter]) {
			continue;
		}

		std::pair<double, double> score = get_young_score(
				papers, counter, num_candidates,
				num_ballots, hopefuls, is_relaxed, is_sym_comp,
				false);

		if (is_relaxed) {
			assert(score.first != -1);
		} else	{
			assert(score.second != -1);
		}

		if (is_relaxed) {
			toRet.insert(candscore(counter, score.first));
		} else	{
			toRet.insert(candscore(counter, score.second));
		}
	}

	return (std::pair<ordering, bool>(toRet, false));
}

young::young(bool use_sym_comp, bool use_relax) {
	is_sym_comp = use_sym_comp;
	is_relaxed = use_relax;

	cached_name = determine_name();
}

