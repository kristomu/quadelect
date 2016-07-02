// The actual Offense-Defense model of Govan et al, but with multiplication by
// arctan instead of division.

#include "odm_atan.h"

using namespace std;

double odm_atan::nltrans(double pairwise_value, 
		double opposing_strength) const {
	return (pairwise_value * atan(opposing_strength));
}

double odm_atan::get_score(double offense, double defense) const {
	return (offense / defense);
}

string odm_atan::odm_name() const {
	return("ODM-arctan");
}
