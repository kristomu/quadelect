
#include "quotas.h"

// Common operations for quota-based methods like STV and Quota Bucklin.

double get_quota(const election_t & ballots,
	size_t num_remaining_seats, double adjustment) {

	// Sum of ballot weights.
	double num_voters = 0;

	for (auto pos = ballots.begin(); pos != ballots.end(); ++pos) {
		num_voters += pos->get_weight();
	}

	// Droop quota. Hare is better but more vulnerable to vote mgmt.
	// TODO: Make this tunable too.

	return num_voters / (num_remaining_seats + adjustment);
}

double get_droop_quota(const election_t & ballots,
	size_t num_remaining_seats) {

	return get_quota(ballots, num_remaining_seats, 1);
}

double get_hare_quota(const election_t & ballots,
	size_t num_remaining_seats) {

	return get_quota(ballots, num_remaining_seats, 0);
}