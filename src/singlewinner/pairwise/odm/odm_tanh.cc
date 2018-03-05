// The actual Offense-Defense model of Govan et al, but with multiplication by
// tanh instead of division.

#include "odm_tanh.h"

using namespace std;

double odm_tanh::nltrans(double pairwise_value, 
		double opposing_strength) const {
	return (pairwise_value * tanh(opposing_strength));
}

double odm_tanh::get_score(double offense, double defense) const {
	return (offense / defense);
}

string odm_tanh::odm_name() const {
	return("ODM-tanh");
}
