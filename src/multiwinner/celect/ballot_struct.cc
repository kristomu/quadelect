#include "ballot_struct.h"

// Should be moved to another file
bool operator < (const positional_count & x, const positional_count & y) {
	return (x.score < y.score);
}
