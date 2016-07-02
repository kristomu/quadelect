// The Hypertext Induced Text Search (HITS) algorithm. 

#include "hits.h"

using namespace std;

double hits::nltrans(double pairwise_value, double opposing_strength) const {
	return (pairwise_value * opposing_strength);
}

void hits::ir_norm(vector<double> & factors) const {

	double sum = 0;

	for (int counter = 0; counter < factors.size(); ++counter)
		sum += factors[counter] * factors[counter];

	sum = sqrt(sum);

	for (int counter = 0; counter < factors.size(); ++counter)
		factors[counter] /= sum;
}

double hits::get_score(double offense, double defense) const {
	return (offense); // Since they converge, o/d would be one.
}

string hits::odm_name() const {
	return("HITS");
}

