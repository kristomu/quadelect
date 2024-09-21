#include <vector>

#ifndef __BALLOT_STRUCT
#define __BALLOT_STRUCT

typedef struct ballot_component {
	int candidate_reference;
	int rank;
};

// for positional voting systems
typedef struct positional_count {
	int candidate_reference;
	double score;
};

typedef double m_num;

typedef struct ballot_bunch {
	m_num multiplier;		// how many voted this way.
	// non-int for geographic PR, but
	// should be fixed point or Rational.

	std::vector<ballot_component> ballot;
};

bool operator < (const positional_count & x, const positional_count & y);

#endif // __BALLOT_STRUCT
