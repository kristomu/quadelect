// The actual Offense-Defense model of Govan et al.

#include "odm.h"

double odm::nltrans(double pairwise_value, double opposing_strength) const {
	return (pairwise_value / opposing_strength);
}

double odm::get_score(double offense, double defense) const {
	return (offense / defense);
}

string odm::odm_name() const {
	return("ODM");
}
