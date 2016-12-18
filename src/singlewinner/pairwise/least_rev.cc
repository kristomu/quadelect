// A family of Condorcet methods based around Least Reversal ("Plain 
// Condorcet"). The methods are:
//	L-R offense: sum of victories against other candidates
//	L-R defense: sum of defeats against the candidate, negated
//	L-R both: the former plus the latter ("margin").
//
// It also supports fractional powers for the summation, even with negative
// matrix components, in which case we use a complex number and take the
// absolute value, negated if the real component is less than zero. This is
// slow in the general case, though, so we only do it after we detect a negative
// value and we're using a fractional power.

#include "method.h"
#include "../method.h"
#include "../../pairwise/matrix.h"

#include <complex>

#include "least_rev.h"

using namespace std;

pair<ordering, bool> least_rev::pair_elect(const abstract_condmat & input,
		const vector<bool> & hopefuls, cache_map & cache,
		bool winner_only) const {

	ordering toRet;

	complex<double> comp_score(0, 0), cpwr(power, 0);
	double real_score, cur_off, cur_def;

	for (int counter = 0; counter < input.get_num_candidates(); ++counter) {
		if (!hopefuls[counter]) continue;

		comp_score = 0;	// 0 + 0i
		real_score = 0;

		for (int sec = 0; sec < input.get_num_candidates(); ++sec) {
			if (counter == sec || !hopefuls[sec]) continue;

			if (offense)
				cur_off = input.get_magnitude(counter, sec,
						hopefuls);
			else	cur_off = 0;
			
			if (defense)
				cur_def = input.get_magnitude(sec, counter,
						hopefuls);
			else	cur_def = 0;

			// If we have to use the complexnum, do so. Cast power
			// to complex so it knows to use the complex power
			// function.
			if ( (cur_def < 0 || cur_off < 0) && power != 
					round(power))
				comp_score += pow(cur_off, cpwr) - 
					pow(cur_def, cpwr);
			else 
				real_score += spow(cur_off, power) -
					spow(cur_def, power);
		}

		// Combine real and complex to get a score.
		comp_score += real_score;

		// Transform it in some way that's unitary if there's no 
		// imaginary part, yet distinguishes negative and positive
		// scores.
		if (comp_score.real() < 0)
			real_score = -abs(comp_score);
		else	real_score =  abs(comp_score);

		// Finally, set the candidate to have this score.
		toRet.insert(candscore(counter, real_score));
	}

	return(pair<ordering, bool>(toRet, false));
}

string least_rev::pw_name() const {
	string stub;

	if (offense && defense)
		stub = "L-R both/";
	else {
		if (offense)
			stub = "L-R offense/";
		else	stub = "L-R defense/";
	}

	return(stub + dtos(power));
}

least_rev::least_rev(pairwise_type def_type_in) : pairwise_method(def_type_in) {
	power = 1;
	offense = false;
	defense = true;
	update_name();
}

least_rev::least_rev(pairwise_type def_type_in, bool off_in, bool def_in, 
		double power_in) : pairwise_method(def_type_in) {
	// If neither offense nor defense is specified, bomb, as we can't
	// make a method out of that.
	assert (off_in || def_in);

	power = power_in;
	offense = off_in;
	defense = def_in;
	update_name();
}

void least_rev::set_power(double power_in) {
	power = power_in;
	update_name();
}

bool least_rev::set_approach(bool offense_in, bool defense_in) {
	if (!offense_in && !defense_in)
		return(false);

	offense = offense_in;
	defense = defense_in;
	update_name();

	return(true);
}

