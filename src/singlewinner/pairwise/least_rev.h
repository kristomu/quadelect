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

#ifndef _VOTE_P_LR
#define _VOTE_P_LR

#include "method.h"
#include "../method.h"
#include "../../pairwise/matrix.h"

using namespace std;

class least_rev : public pairwise_method {
	private:
		bool offense, defense;
		double power;

		// Shortcut pow: if num is 0, just abort
		double spow(const double & num, const double & exponent) const;

	public:
		pair<ordering, bool> pair_elect(const abstract_condmat & input,
				const vector<bool> & hopefuls,
				cache_map * cache, bool winner_only) const;
		string pw_name() const;
		least_rev(pairwise_type def_type_in);
		least_rev(pairwise_type def_type_in, bool offense, bool defense,
				double power_in);

		void set_power(double power_in);
		bool set_approach(bool offense_in, bool defense_in);
		double get_power() const { return(power); }
		bool is_offense() const { return(offense); }
		bool is_defense() const { return(defense); }
};

// TODO: Make something better in tools.h / tools.cc using exponentiation by
// squaring.
inline double least_rev::spow(const double & num, 
		const double & exponent) const {
	if (num == 0) return(0);
	if (num == 1) 
		return(num);
	else	return(pow(num, exponent));
}

#endif
