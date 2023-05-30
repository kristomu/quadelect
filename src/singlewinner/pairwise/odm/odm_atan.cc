// The actual Offense-Defense model of Govan et al, but with multiplication by
// arctan instead of division.

#include "odm_atan.h"


double odm_atan::nltrans(double pairwise_value,
	double opposing_strength) const {
	return (pairwise_value * atan(opposing_strength));
}

double odm_atan::get_score(double offense, double defense) const {
	if (offense == 0 && defense == 0) {
		return 0;    // hack
	}

	return (offense / defense);
}

std::string odm_atan::odm_name() const {
	return ("ODM-arctan");
}
