#include "binary.h"

#include "multiwinner/helper/errors.h"

void binary_proportionality::prepare(const positions_election & p_e) {

	size_t num_voters = p_e.voters_pos.size(),
		   num_issues = p_e.voters_pos[0].size();

	issue_voter_proportions.resize(num_issues);
	issue_winner_proportions.resize(num_issues);

	std::fill(issue_voter_proportions.begin(),
		issue_voter_proportions.end(), 0.0);

	candidate_opinions = p_e.candidates_pos;

	for (size_t voter = 0; voter < num_voters; ++voter) {
		for (size_t issue = 0; issue < num_issues; ++issue) {
			if (p_e.voters_pos[voter][issue] != 0 &&
				p_e.voters_pos[voter][issue] != 1) {
				// Maybe use sign value here to generalize to any
				// distribution??? Would that be too confusing?
				// Would it be helpful? Nah, that would make Bernoulli
				// fail -- silently. Probably not a good idea.
				throw std::invalid_argument("Binary proportionality: voter"
					" issue vectors aren't binary!");
			}
			if (p_e.voters_pos[voter][issue] == 1) {
				issue_voter_proportions[issue] += 1.0/num_voters;
			}
		}
	}
}

double binary_proportionality::get_error(
	const council_t & outcome) {

	size_t num_seats = outcome.size(),
		   num_issues = issue_winner_proportions.size();

	std::fill(issue_winner_proportions.begin(),
		issue_winner_proportions.end(), 0.0);

	for (size_t winner: outcome) {
		for (size_t issue = 0; issue < num_issues; ++issue) {
			if (candidate_opinions[winner][issue] != 0 &&
				candidate_opinions[winner][issue] != 1) {
				// Maybe use sign value here to generalize to any
				// distribution??? Would that be too confusing?
				// Would it be helpful? Nah, that would make Bernoulli
				// fail -- silently. Probably not a good idea.
				throw std::invalid_argument("Binary proportionality: winner"
					" issue vectors aren't binary!");
			}
			if (candidate_opinions[winner][issue] == 1) {
				issue_winner_proportions[issue] += 1.0/num_seats;
			}
		}
	}

	return sli(issue_winner_proportions,
			issue_voter_proportions);
}