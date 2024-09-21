#pragma once

#include "common/ballots.h"

// Common operations for quota-based methods like STV and Quota Bucklin.

double get_droop_quota(const election_t & ballots,
	size_t num_remaining_seats);

double get_hare_quota(const election_t & ballots,
	size_t num_remaining_seats);