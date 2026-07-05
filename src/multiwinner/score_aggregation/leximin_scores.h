// Class for lexicographically comparing minimal scores.
// This is used for (will be used for) keeping track of optima
// for egalitarian optimizing methods.

#include <vector>
#include <map>

#include "score_aggregate.h"

// Using a vector and then sorting *might* be faster; find out later.

class leximin_scores : public score_aggregate {
	private:
		// The map's key (primary) is the score value and the value (secondary)
		// is the weight (i.e. number of scores with that score value).
		std::map<double, double> entries;

		ordinal three_way_compare(
			const leximin_scores & other) const;

	public:
		bool operator<(const leximin_scores & other) const;
		bool operator<=(const leximin_scores & other) const;
		bool operator>(const leximin_scores & other) const;
		bool operator>=(const leximin_scores & other) const;
		bool operator==(const leximin_scores & other) const;
		bool operator!=(const leximin_scores & other) const;

		void insert(double weight, double score);
};