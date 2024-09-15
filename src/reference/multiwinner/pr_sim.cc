// Testing party list PR - put this elsewhere at some point
// I want to reassure myself that the SLI can determine departures
// from proportionality in both directions (small-state and large-state
// biases).

#include <algorithm>
#include <cmath>
#include <iterator>
#include <iostream>
#include <numeric>
#include <vector>

#include <stdexcept>

#include "party_list.h"

#include "../../lib/combinations/combinations.h"

// Sainte-Lague index
double evaluate(const party_list_result & result,
	const std::vector<double> & support_proportions) {

	double sli = 0;

	for (size_t i = 0; i < support_proportions.size(); ++i) {
		double observed = result.seats[i]/(double)result.total_num_seats;
		double expected = support_proportions[i];

		if (observed == 0 && expected == 0) {
			continue;
		}

		sli += pow(observed - expected, 2)/expected;
	}

	return sli;
}

/* Divisor methods, using the Webster formulation. */

party_list_result get_council_inner(double rounding_bias,
	double factor, const std::vector<double> & support_proportions) {

	party_list_result out;

	for (size_t i = 0; i < support_proportions.size(); ++i) {
		size_t seats_here = ceil(support_proportions[i] * factor - rounding_bias);
		out.total_num_seats += seats_here;
		out.seats.push_back(seats_here);
	}

	return out;
}

party_list_result get_council(double rounding_bias,
	const std::vector<double> & support_proportions,
	size_t num_seats) {

	// Bisection search
	size_t num_parties = support_proportions.size();

	double lower = num_seats * rounding_bias,
		   upper = num_seats + rounding_bias * num_parties;

	while (upper - lower > 1e-9) {
		double middle = (lower + upper) * 0.5;

		size_t seats_got = get_council_inner(rounding_bias,
				middle, support_proportions).total_num_seats;

		if (seats_got == num_seats) {
			return get_council_inner(rounding_bias, middle,
					support_proportions);
		}

		if (seats_got > num_seats) {
			upper = middle;
		} else {
			lower = middle;
		}
	}

	throw std::runtime_error("Couldn't find proper divisor!");
}

/* End of divisor methods */

/* Reimplementation of Harmonic voting, as it shows a pretty strong
   "Sainte-Laguë is not optimal" effect in my main implementation and
   model. */

// one rating per candidate, with ratings summing up to one.
typedef std::vector<double> rated_ballot;

// Assignment of candidates to seats: an s-vector of candidate
// indices.
typedef std::vector<size_t> assignment;

double harmonic(double rounding_bias, const
	std::vector<std::vector<double> > & rated_ballots,
	assignment::const_iterator begin,
	assignment::const_iterator end) {

	// Contains the list of ratings for the chosen
	// candidates, sorted from greatest to least.
	size_t council_size = end - begin;
	std::vector<double> sorted_viable_ratings(council_size, 0);

	double quality = 0;

	for (const std::vector<double> & ballot: rated_ballots) {
		size_t i = 0;
		for (auto pos = begin; pos != end; ++pos) {
			// That ballot's rating of the ith proposed candidate.
			sorted_viable_ratings[i++] = ballot[*pos];
		}

		std::sort(sorted_viable_ratings.begin(),
			sorted_viable_ratings.end(), std::greater<double>());

		for (i = 0; i < council_size; ++i) {
			// Warren says the denominator is j-1+delta, but
			// we're indexing from zero, so it effectively just
			// becomes delta.
			quality += sorted_viable_ratings[i] / (i + rounding_bias);
		}
	}

	return quality; // To be maximized.
}

assignment get_optimal_council(double rounding_bias, const
	std::vector<std::vector<double> > & rated_ballots,
	size_t council_size) {

	// Each ballot rates every candidate.
	size_t num_candidates = rated_ballots[0].size();

	std::vector<size_t> v(num_candidates);
	std::iota(v.begin(), v.end(), 0);

	double quality = 0;
	assignment best_council;

	for_each_combination(v.begin(), v.begin() + council_size,
		v.end(), [rounding_bias, rated_ballots, &quality, &best_council]
		(assignment::const_iterator begin,
	assignment::const_iterator end) -> bool {

		double quality_this = harmonic(rounding_bias,
			rated_ballots, begin, end);

		if (quality_this > quality) {
			quality = quality_this;
			best_council = assignment(begin, end);
		}

		return false; // keep going
	}
	);

	return best_council;
}

// Get the support of each party by the voters.
std::vector<double> get_party_support(size_t num_parties) {

	std::vector<double> support;
	double total_support = 0;

	for (size_t i = 0; i < num_parties; ++i) {
		double support_this_party = drand48();
		support.push_back(support_this_party);
		total_support += support_this_party;
	}

	// Make the support probabilities sum to one.
	for (size_t i = 0; i < support.size(); ++i) {
		support[i] /= total_support;
	}

	// Make it easier to see small vs large party bias by
	// putting the small parties first.
	std::sort(support.begin(), support.end());

	return support;
}

class voter_opinions {
	public:
		std::vector<std::vector<bool> > opinions;
		std::vector<std::vector<bool> > candidate_opinions;

		std::vector<double> opinion_profile;
};

voter_opinions get_party_voting_instance(size_t num_voters,
	size_t num_parties, size_t council_size) {

	// We want as many candidates per party as there are seats.
	// Opinion space has as many dimensions as there are parties,
	// and every candidate has a positive opinion of his own party.

	size_t party, candidate, voter;
	size_t num_issues = num_parties;

	voter_opinions output;

	for (party = 0; party < num_parties; ++party) {
		std::vector<bool> opinion(num_issues, false);
		opinion[party] = true;

		for (candidate = 0; candidate < council_size; ++candidate) {
			output.candidate_opinions.push_back(opinion);
		}
	}

	std::vector<double> support = get_party_support(num_parties);

	// Try to approximate support given the number of voters. This
	// could be made more rigorous using Webster itself, but I'll try
	// to reproduce the logic of the main multiwinner simulator
	// accurately before I do anything like that.

	std::vector<size_t> voters_supporting_party(num_parties, 0);

	std::copy(support.begin(), support.end(),
		std::ostream_iterator<double>(std::cout, " "));
	std::cout << "\n";

	for (voter = 0; voter < num_voters; ++voter) {
		// Roulette selection
		double p = drand48();
		double cdf_so_far = 0;

		party = 0;

		while (party < num_parties && cdf_so_far < p) {
			cdf_so_far += support[party];
			if (cdf_so_far < p) {
				++party;
			}
		}

		std::vector<bool> opinion(num_issues, false);
		opinion[party] = true;
		output.opinions.push_back(opinion);
		++voters_supporting_party[party];
	}

	for (party = 0; party < num_parties; ++party) {
		support[party] = voters_supporting_party[party]/(double)num_voters;
	}

	output.opinion_profile = support;

	return output;
}

party_list_result get_party_voting_result(const assignment &
	method_outcome,
	size_t num_candidates, size_t num_parties) {

	party_list_result output;
	output.seats = std::vector<size_t>(num_parties, 0);
	output.total_num_seats = method_outcome.size();

	if (num_candidates % num_parties != 0) {
		throw std::logic_error("Number of candidates must be a"
			" multiple of number of parties!");
	}

	for (size_t elected: method_outcome) {
		output.seats[elected/num_parties]++;
	}

	return output;
}

size_t hamming_equality(const std::vector<bool> & a,
	const std::vector<bool> & b) {

	if (a.size() != b.size()) {
		throw std::invalid_argument("Hamming equality: equal "
			"size vectors, please!");
	}

	size_t agreements = 0;

	for (size_t i = 0; i < a.size(); ++i) {
		if (a[i] == b[i]) {
			++agreements;
		}
	}

	return agreements;
}

std::vector<double> normalize(std::vector<double> in) {

	double maximum = *std::max_element(in.begin(), in.end()),
		   minimum = *std::min_element(in.begin(), in.end());

	for (size_t i = 0; i < in.size(); ++i) {
		in[i] = (in[i] - minimum) / (maximum - minimum);
	}

	return in;
}

std::string bool_vector_str(const std::vector<bool> & vec) {
	std::string out;
	for (bool x: vec) {
		if (x) {
			out += "#";
		} else {
			out += ".";
		}
	}

	return out;
}

std::vector<std::vector<double> > get_rated_ballots(
	const voter_opinions & vo) {

	std::vector<std::vector<double> > ballots;

	for (size_t voter = 0; voter < vo.opinions.size(); ++voter) {
		std::vector<double> this_ballot;

		/*std::cout << "voter " << voter << ": opinion " <<
			bool_vector_str(vo.opinions[voter]) << ": ";*/

		for (size_t candidate = 0;
			candidate < vo.candidate_opinions.size(); ++candidate) {
			size_t rating = hamming_equality(vo.opinions[voter],
					vo.candidate_opinions[candidate]);
			this_ballot.push_back(rating
				+ drand48() * 0.001);
			//std::cout << rating << " ";
		}
		ballots.push_back(normalize(this_ballot));
		//ballots.push_back(this_ballot);
		/*std::cout << "\n";
		std::vector<double> normalized = normalize(this_ballot);
		std::cout << "\tAfter normalization: ";
		std::copy(normalized.begin(), normalized.end(), std::ostream_iterator<double>(std::cout, " "));
		std::cout << "\n";*/
	}

	return ballots;
}

// The output, printed to screen, should show least Sainte-Lague index
// error for bias = 0.5.
void test_sli_implementation() {
	size_t bias_bins = 10;

	std::vector<double> sum_sli(bias_bins+1, 0);
	size_t rounds = 10000;

	for (size_t round = 0; round < rounds; ++round) {
		size_t num_parties = 5, num_seats = 5;

		std::vector<double> support = get_party_support(
				num_parties);

		for (int bias_idx = 0; bias_idx <= bias_bins; ++bias_idx) {

			double rounding_bias = bias_idx / (double)bias_bins;

			party_list_result output = get_council(rounding_bias, support,
					num_seats);

			/*std::cout << "Bias: " << rounding_bias << "\t";
			std::copy(output.seats.begin(), output.seats.end(),
				std::ostream_iterator<size_t>(std::cout, " "));
			std::cout << "\t" << "Sainte-Lague index: "
				<< evaluate(output, support) << "\n";*/
			sum_sli[bias_idx] += evaluate(output, support);
		}
	}

	std::cout << "\n\nSUMMARY\n";

	for (int bias_idx = 0; bias_idx <= bias_bins; ++bias_idx) {
		double rounding_bias = bias_idx / (double) bias_bins;

		std::cout << rounding_bias << " " <<
			sum_sli[bias_idx]/(double)rounds << "\n";
	}
}

// Create a faux "opinion space" of n dimensions for n parties,
// a number of candidates per party, and voters who are aligned
// with one party. Then check that the whole stack: turning voter
// and candidate opinions into rated ballots, and running harmonic
// voting, returns the same results as the divisor method with the
// same bias.

// It's currently known that delta=0 and delta=1 give diverging
// results; so we should properly only trust 0 < delta < 1.

void test_harmonic(std::vector<double> & out_sli_results,
	std::vector<size_t> & out_tests_performed,
	size_t max_bias_bin) {

	size_t num_voters = 20, num_parties = 5, num_seats = 5;

	voter_opinions opinion_instance = get_party_voting_instance(
			num_voters, num_parties, num_seats);

	std::vector<std::vector<double> > rated_ballots =
		get_rated_ballots(opinion_instance);

	// For Harmonic voting, delta = 0 and delta = 1 behave differently
	// than their respective party-list methods.
	for (size_t bias_bin = 1; bias_bin < max_bias_bin; ++bias_bin) {

		double rounding_bias = bias_bin / (double) max_bias_bin;

		// Get a reference pary list allocation to check if there's a
		// valid unique allocation for this bias value, and to later
		// check if Harmonic is actually doing what we expect.

		party_list_result direct_party_list_result;

		try {
			direct_party_list_result = get_council(
					rounding_bias, opinion_instance.opinion_profile,
					num_seats);
		} catch (std::runtime_error & e) {
			continue;
		}

		assignment optimal_council = get_optimal_council(rounding_bias,
				rated_ballots, num_seats);

		std::cout << "Elected: ";
		std::copy(optimal_council.begin(), optimal_council.end(),
			std::ostream_iterator<size_t>(std::cout, " "));
		std::cout << "\n";

		party_list_result pr_result = get_party_voting_result(
				optimal_council, rated_ballots[0].size(), num_parties);

		std::cout << "Party allocation: ";
		std::copy(pr_result.seats.begin(), pr_result.seats.end(),
			std::ostream_iterator<size_t>(std::cout, " "));
		std::cout << "\n";

		if (pr_result.seats != direct_party_list_result.seats) {
			throw std::runtime_error("Harmonic voting: Discrepancy "
				"with party list outcome detected!");
		}

		out_sli_results[bias_bin] += evaluate(pr_result,
				opinion_instance.opinion_profile);
		++out_tests_performed[bias_bin];

		std::cout << " Test: " << evaluate(pr_result,
				opinion_instance.opinion_profile) << "\n";
	}
}

/* End of harmonic voting party list verification */


/* Beginning of reproduction of the main simulator approach. */
/* Some parts differ slightly: for instance, we don't let the
   candidates vote, as real elections have tons of voters and
   few candidates. */

voter_opinions get_general_voting_instance(size_t num_voters,
	size_t num_candidates, size_t num_issues, size_t council_size) {

	// We want as many candidates per party as there are seats.
	// Opinion space has as many dimensions as there are parties,
	// and every candidate has a positive opinion of his own party.

	size_t candidate, voter, issue;

	voter_opinions output;

	// Set the rough popularity of "yes" on each opinion axis.
	for (issue = 0; issue < num_issues; ++issue) {
		output.opinion_profile.push_back(/*drand48()*/0.1);
	}

	// Set the voters' opinions.
	for (voter = 0; voter < num_voters; ++voter) {
		std::vector<bool> voter_opinion(num_issues, false);

		for (issue = 0; issue < num_issues; ++issue) {
			voter_opinion[issue] = (drand48() < output.opinion_profile[issue]);
		}
		output.opinions.push_back(voter_opinion);
	}

	// Update popularity.
	for (issue = 0; issue < num_issues; ++issue) {
		size_t agreeing_voters = 0;
		for (voter = 0; voter < num_voters; ++voter) {
			if (output.opinions[voter][issue]) {
				++agreeing_voters;
			}
		}
		output.opinion_profile[issue] = agreeing_voters/(double)num_voters;
	}

	// Set the candidates' opinions.
	for (candidate = 0; candidate < num_candidates; ++candidate) {
		std::vector<bool> cand_opinion(num_issues, false);

		for (issue = 0; issue < num_issues; ++issue) {
			cand_opinion[issue] = (drand48() < output.opinion_profile[issue]);
		}
		output.candidate_opinions.push_back(cand_opinion);
	}

	return output;
}

double evaluate_sli(const voter_opinions & opinion_instance,
	assignment outcome) {

	size_t num_candidates = opinion_instance.candidate_opinions.size();
	size_t num_issues = opinion_instance.opinion_profile.size();
	size_t issue;

	std::vector<double> elected_opinion_profile(num_issues, 0);

	// Get the opinion profile of the elected candidates.
	for (size_t elected_cand_idx: outcome) {
		for (issue = 0; issue < num_issues; ++issue) {
			if (opinion_instance.candidate_opinions[elected_cand_idx][issue]) {
				++elected_opinion_profile[issue];
			}
		}
	}

	for (issue = 0; issue < num_issues; ++issue) {
		elected_opinion_profile[issue] /= (double)num_candidates;
	}

	double sli = 0;

	for (issue = 0; issue < num_issues; ++issue) {
		double observed = elected_opinion_profile[issue];
		double expected = opinion_instance.opinion_profile[issue];

		if (observed == 0 && expected == 0) {
			continue;
		}

		sli += pow(observed - expected, 2)/expected;
	}

	return sli;
}


// Now try generating more complex opinion profiles. Are we going to
// get the same strange lower-bias preference as in the main simulator?

double evaluate_bias(const voter_opinions & opinion_instance,
	assignment outcome);

void test_harmonic_composite(std::vector<double> & out_sli_results,
	std::vector<size_t> & out_tests_performed,
	size_t max_bias_bin) {

	size_t num_voters = 20, num_parties = 5, num_seats = 5,
		   num_issues = 4, num_candidates = 8, council_size = 5;

	voter_opinions opinion_instance = get_general_voting_instance(
			num_voters, num_candidates, num_issues, council_size);

	std::vector<std::vector<double> > rated_ballots =
		get_rated_ballots(opinion_instance);

	for (size_t bias_bin = 1; bias_bin < max_bias_bin; ++bias_bin) {

		double rounding_bias = bias_bin / (double) max_bias_bin;

		assignment optimal_council = get_optimal_council(rounding_bias,
				rated_ballots, num_seats);

		double sli_result = evaluate_sli(opinion_instance,
				optimal_council);

		out_sli_results[bias_bin] += sli_result;
		++out_tests_performed[bias_bin];

		/*std::cout << "Bias: " << rounding_bias << " Test: "
			<< sli_result << " " << evaluate_bias(opinion_instance,
				optimal_council) << "\n";*/
	}
}

// IDEA: Determine bias this way:

// Let x_i be the fraction of the voters who agree with issue i.
// Let y_i be the number of seats held by candidates who agree with issue i,
//			divided by x_i, i.e. the number of seats per voter, times a
//			constant.
// Use Spearman's rank correlation between x_i and y_i. If large x_i predict
// large y_i then we have a "big opinion bias" and vice versa.

// We should also determine if it's possible to choose candidates so that this
// is unbiased. If not, then the model itself may be biased.

// I'm going to need some kind of anchor point to determine if Harmonic is
// simply bad or if something else is going on.

// https://wikimedia.org/api/rest_v1/media/math/render/svg/5984dfb290912b0e0b92a984bf49cdd628c38b2c

double correlation(std::vector<double> & x, std::vector<double> & y) {

	if (x.size() != y.size()) {
		throw std::runtime_error("Correlation: equal sized vectors, please.");
	}

	size_t i;

	double EX = std::accumulate(x.begin(), x.end(), 0.0)/x.size();
	double EY = std::accumulate(y.begin(), y.end(), 0.0)/y.size();
	double EXY = 0, EXsq = 0, EYsq = 0;

	for (i = 0; i < x.size(); ++i) {
		EXY += x[i] * y[i];
		EXsq += x[i] * x[i];
		EYsq += y[i] * y[i];
	}

	EXY /= x.size();
	EXsq /= x.size();
	EYsq /= y.size();

	return (EXY - EX * EY) / (sqrt(EXsq - EX*EX) * sqrt(EYsq - EY*EY));
}

double rank_correlation(std::vector<double> & x, std::vector<double> & y) {

	std::vector<std::pair<double, size_t> > xp, yp;

	for (size_t i = 0; i < x.size(); ++i) {
		xp.push_back({x[i], i});
		yp.push_back({y[i], i});
	}

	std::sort(xp.begin(), xp.end());
	std::sort(yp.begin(), yp.end());

	std::vector<double> x_rank, y_rank;

	for (size_t i = 0; i < x.size(); ++i) {
		x_rank.push_back(xp[i].second);
		y_rank.push_back(yp[i].second);
	}

	return correlation(x_rank, y_rank);
}

// Let x_i be the fraction of the voters who agree with issue i.
// Let y_i be the number of seats held by candidates who agree with issue i,
//			divided by x_i, i.e. the number of seats per voter, times a
//			constant.
// Use Spearman's rank correlation between x_i and y_i. If large x_i predict
// large y_i then we have a "big opinion bias" and vice versa.


// UNTESTED, I should confirm Warren's results for pure party list first...
// Or check with some example/test vectors.
double evaluate_bias(const voter_opinions & opinion_instance,
	assignment outcome) {

	size_t num_issues = opinion_instance.opinion_profile.size();

	std::vector<double> x = opinion_instance.opinion_profile;
	std::vector<double> y(num_issues, 0);

	for (size_t candidate_idx: outcome) {
		for (size_t issue = 0; issue < num_issues; ++issue) {
			if (opinion_instance.candidate_opinions[candidate_idx][issue]) {
				++y[issue];
			}
		}
	}

	for (size_t issue = 0; issue < num_issues; ++issue) {
		y[issue] /= x[issue];
	}

	return rank_correlation(x, y);
}

int main() {
	size_t max_bias_bin = 20;

	std::vector<double> sli_results(max_bias_bin+1);
	std::vector<size_t> tests_performed(max_bias_bin+1);

	for (int i = 0; i < 10000; ++i) {
		std::cout << "==== iteration " << i << " ====\n";
		test_harmonic_composite(sli_results, tests_performed,
			max_bias_bin);
	}

	for (size_t bias_bin = 0; bias_bin <= max_bias_bin; ++bias_bin) {
		double rounding_bias = bias_bin / (double) max_bias_bin;

		std::cout << "Bias: " << rounding_bias << "\t";

		if (tests_performed[bias_bin] == 0) {
			std::cout << "N/A\n";
		} else {
			std::cout << sli_results[bias_bin]/tests_performed[bias_bin] << "\n";
		}
	}
}
