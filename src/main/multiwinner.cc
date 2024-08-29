
// Multiwinner hacks.
// Segment properly later. TODO.

// Also TODO aggregate ballots in the case that multiple voters vote in the
// same way. Sketch of how to do this: Make map from orderings to int,
// map[this ordering]++ for all ballots, then adjust weights accordingly.
// Remember to delete the first and second keys of the map properly, or it'll
// leak.

// TODO: Write normalized array contents to files so that we can make graphs
// based on them, etc.
// Wishlist: Candidate ordering by opinions. FindBest/FindWorst instead of
// best/worst-of-10000. Ballot aggregation/ballot trees. Clues.

#include "common/ballots.h"
#include "singlewinner/positional/all.h"
#include "singlewinner/pairwise/all.h"
#include "singlewinner/elimination/elimination.h"
#include "singlewinner/stats/cardinal.h"

/*#include "tools.cc"
#include "mwstats.cc"
#include "condorcet/methods.cc"*/

#include "stats/multiwinner/mwstats.h"
#include "multiwinner/helper/errors.cc"

#include "multiwinner/methods.h"
#include "multiwinner/exhaustive/birational.h"
#include "multiwinner/exhaustive/isoelastic.h"
#include "multiwinner/exhaustive/lpv.h"

#include "multiwinner/psc.h"
#include "multiwinner/qbuck.h"
#include "multiwinner/meek_stv.h"
#include "multiwinner/shuntsstv.h"
#include "multiwinner/compat_qbuck.h"
#include "multiwinner/dhwl.h"
#include "multiwinner/stv.h"
//#include "multiwinner/qpq.cc"

#include "hack/msvc_random.h"

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <set>
#include <assert.h>

#define errm rmse

using namespace std;

// Opinion profiles. Binary for now, so we can check validity - then 2xdouble
// (one for position on scale, one for intensity) later.

vector<bool> generate_opinion_profile(const vector<double> & bias) {

	vector<bool> toRet(bias.size());

	for (size_t counter = 0; counter < bias.size(); ++counter) {
		toRet[counter] = (drand48() < bias[counter]);
	}

	return (toRet);
}

string get_printable_profile(const vector<bool> & profile) {

	string output;

	for (size_t counter = 0; counter < profile.size(); ++counter)
		if (profile[counter]) {
			output += "#";
		} else	{
			output += "-";
		}

	return (output);
}

void print_profiles(const vector<vector<bool> > & profiles, size_t begin,
	size_t end) {

	for (size_t counter = begin; counter < end; ++counter) {
		cout << counter << "\t" << get_printable_profile(profiles[counter]) <<
			endl;
	}
}

vector<double> sum_opinion_profile(const vector<vector<bool> > &
	population_profiles) {

	size_t population_size = population_profiles.size();

	vector<double> opinion_count(population_profiles[0].size(), 0);

	size_t counter, sec;

	for (counter = 0; counter < population_size; ++counter)
		for (sec = 0; sec < opinion_count.size(); ++sec)
			if (population_profiles[counter][sec]) {
				++opinion_count[sec];
			}

	for (counter = 0; counter < opinion_count.size(); ++counter) {
		opinion_count[counter] /= (double)population_size;
	}

	return (opinion_count);
}

vector<double> get_quantized_opinion_profile(const list<int> & candidates,
	const vector<vector<bool> > & population_profiles) {

	// Same as above, really, only with a subset.

	vector<double> opinion_count(population_profiles[0].size(), 0);

	size_t counter, sec;

	for (list<int>::const_iterator pos = candidates.begin(); pos !=
		candidates.end(); ++pos)
		for (sec = 0; sec < opinion_count.size(); ++sec)
			if (population_profiles[*pos][sec]) {
				++opinion_count[sec];
			}

	for (counter = 0; counter < opinion_count.size(); ++counter) {
		opinion_count[counter] /= (double)candidates.size();
	}

	return (opinion_count);
}

// The candidates that can be voted upon go from 0 to num_candidates.
// current_index is the population profile index that coresponds to the
// voter in question.

int hamming_distance(const vector<bool> & a, const vector<bool> & b) {

	if (a.size() != b.size()) {
		throw std::invalid_argument("Hamming distance: vectors must be equal length");
	}

	size_t hd = 0;

	for (size_t counter = 0; counter < a.size(); ++counter)
		if (a[counter] ^ b[counter]) {
			++hd;
		}

	return (hd);
}

// TODO later: only rate a subset. But we know that getting a perfect council
// in that case requires hyperproportionality, which we can't have.
ordering construct_ballot_order(const vector<vector<bool> > &
	population_profiles, int num_candidates, int current_index,
	double noise_magnitude) {

	// Say hooray for sets that order themselves! Higher is better, so we
	// take the negative values.
	// For binary profiles, the Hamming distance is a good (optimist)
	// estimate.

	ordering output;

	for (int counter = 0; counter < num_candidates; ++counter) {
		double hamming = hamming_distance(population_profiles
				[current_index], population_profiles[counter]);
		double maxhamming = population_profiles[current_index].size();

		double noise = renorm(0.0, 1.0, drand48(), -noise_magnitude
				* 0.5, noise_magnitude * 0.5);

		double score = (maxhamming + noise_magnitude) - (hamming + noise);

		//score /= maxhamming;

		output.insert(candscore(counter, score));
	}

	return (output);
}

ballot_group construct_ballot(const vector<vector<bool> > &
	population_profiles,
	int num_candidates, int current_index, double noise_magnitude) {

	return (ballot_group(1,
				construct_ballot_order(population_profiles,
					num_candidates, current_index,
					noise_magnitude), true, true));
}

election_t construct_ballots(const vector<vector<bool> > &
	population_profiles, int num_candidates,
	double noise_magnitude) {

	election_t ballots;

	for (size_t counter = 0; counter < population_profiles.size(); ++counter)
		ballots.push_back(construct_ballot(population_profiles,
				num_candidates, counter,
				noise_magnitude));

	return (ballots);
}

// Framing

list<int> random_council(size_t num_candidates, size_t council_size) {

	vector<int> numbered_candidates(num_candidates);

	// Set to 0 to the number of candidates, randomly shuffle and then
	// extract the first n.
	iota(numbered_candidates.begin(), numbered_candidates.end(), 0);
	random_shuffle(numbered_candidates.begin(), numbered_candidates.end());

	list<int> toRet;

	copy(numbered_candidates.begin(), numbered_candidates.begin() +
		council_size, inserter(toRet, toRet.begin()));

	return (toRet);
}

// TODO: Handle case with weighted ballots: 1000 A > B, 1 B > C should have
// only 1/1000 chance of picking B > C, not 1/2.
// Beware: this method is not cloneproof.
list<int> random_ballot(int num_ballots, const election_t & ballots,
	size_t num_candidates, size_t council_size, size_t accept_number) {

	// Read off the first min(ordering size, accept_number) candidates.
	// Then permute randomly and pick the first council_size of these.
	// Inspired by LeGrand's 3OPT minmax approval scheme.


	// Pick the random ballot
	size_t counter = 0, rand_ballot = floor(drand48() * num_ballots);

	election_t::const_iterator pos;

	for (pos = ballots.begin(); pos != ballots.end() && counter <
		rand_ballot; ++pos) {
		++counter;
	}

	// Now pos is a random ballot.
	size_t requested = min(accept_number, pos->contents.size());
	vector<size_t> accepted(requested, 0);
	vector<bool> taken(num_candidates, false); // incomplete ballots

	counter = 0;

	for (ordering::const_iterator opos = pos->contents.begin();
		opos != pos->contents.end() && counter < requested;
		++opos) {
		assert(opos->get_candidate_num() < num_candidates);
		taken[opos->get_candidate_num()] = true;
		accepted[counter++] = opos->get_candidate_num();
	}

	// Fill with random if the voter was too indecisive.
	while (accepted.size() < max(accept_number, council_size)) {
		int proposed_candidate = floor(drand48() * num_candidates);
		if (taken[proposed_candidate]) {
			continue;
		}
		accepted.push_back(proposed_candidate);
		taken[proposed_candidate] = true;
	}

	// Randomly shuffle
	random_shuffle(accepted.begin(), accepted.end());

	// .. and load to the list we're going to return.

	list<int> toRet;

	for (counter = 0; counter < council_size; ++counter) {
		assert(accepted[counter] < num_candidates);
		toRet.push_back(accepted[counter]);
	}

	return (toRet);
}


double get_error(const list<int> & council,
	int num_candidates, size_t council_size,
	const vector<vector<bool> > & population_profiles,
	const vector<double> & whole_pop_profile) {

	assert(council.size() == council_size);

	// Check that there's no double-entry and that the method hasn't
	// elected someone outside of the allowed range.
	vector<bool> seen(num_candidates, false);
	for (list<int>::const_iterator pos = council.begin();
		pos != council.end(); ++pos) {
		assert(0 <= *pos && *pos < num_candidates);
		assert(!seen[*pos]);
		seen[*pos] = true;
	}

	return (errm(whole_pop_profile, get_quantized_opinion_profile(
					council, population_profiles)));
}

/* Majoritarian utility (VSE)
   This nabs the utility from the expressed ballots, which
   may not hold true if we transform them. (It already is only
   approximately true due to the added noise, showing my point.)
*/

std::vector<double> get_utilities(const election_t & election,
	size_t num_candidates) {

	std::vector<double> mean_utilities(num_candidates, 0);
	double total_voting_weight = 0;

	for (const ballot_group & ballot: election) {
		total_voting_weight += ballot.get_weight();

		for (const candscore & cs: ballot.contents) {
			size_t candidate = cs.get_candidate_num();
			mean_utilities[candidate] += cs.get_score();
		}
	}

	for (size_t i = 0; i < num_candidates; ++i) {
		mean_utilities[i] /= total_voting_weight;
	}

	return mean_utilities;
}

double get_random_utilities(const std::vector<double> & utilities) {
	return std::accumulate(utilities.begin(), utilities.end(), 0.0)/
		(double)utilities.size();
}

double get_optimal_utility(std::vector<double> utilities,
	size_t council_size) {
	std::sort(utilities.begin(), utilities.end()); // ascending
	std::reverse(utilities.begin(), utilities.end()); // Now it's descending.

	assert(council_size <= utilities.size());

	double sum_utils = 0;

	for (size_t i = 0; i < council_size; ++i) {
		sum_utils += utilities[i];
	}

	return sum_utils / (double)council_size;
}

double get_council_utility(const std::vector<double> & utilities,
	std::list<int> & council) {

	double sum_utils = 0;

	for (int candidate: council) {
		sum_utils += utilities[candidate];
	}

	return sum_utils / (double)council.size();
}

////

void get_limits(int num_candidates, int council_size,
	const vector<vector<bool> > & population_profiles,
	const vector<double> & whole_pop_profile, int tries,
	double & worst, double & average, double & best) {

	worst = -INFINITY;
	average = 0;
	best = INFINITY;

	double d_worst = 0;

	int counter;

	for (counter = 0; counter < tries; ++counter) {

		double this_error = get_error(random_council(num_candidates,
					council_size), num_candidates,
				council_size, population_profiles,
				whole_pop_profile);

		if (this_error > worst) {
			worst = this_error;
		}
		if (this_error < best) {
			best = this_error;
		}

		average += this_error;

		// Exponential average.
		d_worst = d_worst * 0.5 + this_error * 0.5;

		// Try to normalize the noise in getting best and worst by
		// breaking after a certain error level.
		// Didn't work very well.
		//	if (fabs(this_error - d_worst) < 1e-6) break;

	}

	average /= (double)(counter);
}

// n * tries instead of plain n(as we would have with a vector of ballots.
// TODO: Fix somehow.
double random_ballot(const election_t & ballots,
	int num_candidates, int council_size, int accept_number,
	const vector<vector<bool> > & population_profiles,
	const vector<double> & whole_pop_profile, int tries) {

	int num_ballots = population_profiles.size();

	double sum = 0;

	for (int counter = 0; counter < tries; ++counter)
		sum += get_error(random_ballot(num_ballots, ballots,
					num_candidates, council_size,
					accept_number), num_candidates,
				council_size, population_profiles,
				whole_pop_profile);

	return (sum / (double)tries);
}


// Election method normalization info

/*class multiwinner_stats {
	public:
		multiwinner_method * method;
		double sum_scores;
		double sum_normalized_scores;

		multiwinner_stats(multiwinner_method * method_in) {
			method = method_in;
			sum_scores = 0;
			sum_normalized_scores = 0;
		}
};*/

string padded(string a, int maxlen) {
	int len = max(1, maxlen - (int)a.size());

	return (a + string(len, ' '));
}

string display_stats(double stat_sum, double rounds_so_far,
	double current_unnorm_result, double current_norm_result,
	string name) {

	string toRet = padded(dtos(stat_sum/rounds_so_far), 12) + padded(name,
			30) + "round: " + padded(dtos(current_norm_result),
			9) + " (unnorm: " + padded(dtos(current_unnorm_result), 9) +")";

	return (toRet);
}

/*string display_stats(const multiwinner_stats & in, double rounds_so_far,
		double current_unnorm_result, double current_norm_result) {
	return (display_stats(in.sum_normalized_scores, rounds_so_far,
				current_unnorm_result, current_norm_result,
				in.method->name()));
}*/

// QnD
void print_std_pref(const election_t & f) {

	cout << "STANDARD PREFS:" << endl;

	for (election_t::const_iterator pos = f.begin(); pos != f.end();
		++pos) {
		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos) {
			cerr << (char)('A' + opos->get_candidate_num()) << " ";
		}
		cerr << endl;
	}
}

void set_best(multiwinner_stats & meta, double minimum, double maximum,
	const vector<multiwinner_stats> & source, string search_for_a,
	string search_for_b) {

	vector<double> gathered_scores;

	for (size_t counter = 0; counter < source.size(); ++counter) {
		if (&(source[counter]) == &meta) {
			continue;
		}
		string curname = source[counter].get_name();

		if (curname.find(search_for_a, 0) == std::string::npos) {
			continue;
		}
		if (curname.find(search_for_b, 0) == std::string::npos) {
			continue;
		}
		//	cout << "VV: " << curname << "\t" << curname.find(search_for, 0) << endl;

		gathered_scores.push_back(source[counter].get_last(false));
	}

	sort(gathered_scores.begin(), gathered_scores.end());

	meta.add_result(minimum, gathered_scores[0], maximum);
}

int main(int argc, char * * argv) {

	// Used for inlining.
	int maxnum = 0;
	bool run_forever = false;
	if (argc < 2) {
		std::cerr << "No max match number specified, running forever." << endl;
		run_forever = true;
		maxnum = 1;
	} else {
		maxnum = stoi(argv[1]);
	}

	assert(maxnum > 0);

	// Set up some majoritarian election methods and their stats.
	// Condorcet methods should probably have their own arrays.
	vector<multiwinner_stats> e_methods; // WARNING: Leak. TODO, fix.
	vector<std::shared_ptr<positional> > positional_methods;
	vector<std::shared_ptr<pairwise_method> > condorcet;
	vector<std::shared_ptr<election_method> > other_methods;

	// All are PT_WHOLE for now.
	positional_methods.push_back(std::make_shared<plurality>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<borda>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<antiplurality>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<for_and_against>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<nauru>(PT_WHOLE));
	positional_methods.push_back(std::make_shared<heismantrophy>(PT_WHOLE));
	/*positional_methods.push_back(new baseballmvp(PT_WHOLE));
	positional_methods.push_back(new worstpos(PT_WHOLE));*/

	size_t counter;

	// Then up it by loser elimination..
	for (counter = 0; counter < positional_methods.size(); ++counter)
		other_methods.push_back(std::make_shared<loser_elimination>(
				positional_methods[counter], false,
				true));

	for (counter = 0; counter < positional_methods.size(); ++counter)
		other_methods.push_back(std::make_shared<loser_elimination>(
				positional_methods[counter],
				true, true));

	// Add some it doesn't make any sense to have as *-Elimination or that
	// haven't got this implemented yet.

	/*other_methods.push_back(new bucklin(PT_WHOLE));
	other_methods.push_back(new bucklin(PT_WHOLE, 0.25));
	other_methods.push_back(new qltd(PT_WHOLE));
	other_methods.push_back(new qltd(PT_WHOLE, 0.25));*/

	other_methods.push_back(std::make_shared<cardinal_ratings>(-10, 10,
			false));
	other_methods.push_back(std::make_shared<cardinal_ratings>(-10, 10, true));

	// These are not all Condorcet. Minmin is not.
	// TODO: Better name for the vector.
	condorcet.push_back(std::make_shared<ext_minmax>(CM_MARGINS, true));
	condorcet.push_back(std::make_shared<ext_minmax>(CM_MARGINS, false));
	condorcet.push_back(std::make_shared<schulze>(CM_WV));

	for (counter = 0; counter < condorcet.size(); ++counter) {
		other_methods.push_back(condorcet[counter]);
	}

	// Now derive multiwinner methods off these
	for (counter = 0; counter < positional_methods.size(); ++counter) {
		e_methods.push_back(multiwinner_stats(
				std::make_shared<majoritarian_council>(
					positional_methods[counter])));
	}

	for (counter = 0; counter < other_methods.size(); ++counter) {
		e_methods.push_back(multiwinner_stats(
				std::make_shared<majoritarian_council>(
					other_methods[counter])));
	}

	for (counter = 0; counter < positional_methods.size(); ++counter) {
		e_methods.push_back(multiwinner_stats(
				std::make_shared<addt_ballot_reweighting>(
					positional_methods[counter])));
	}

	// Maybe: IRV-SNTV

	for (counter = 0; counter < condorcet.size(); ++counter) {
		e_methods.push_back(multiwinner_stats(
				std::make_shared<reweighted_condorcet>(
					condorcet[counter])));
	}

	// TODO: Add Meek STV shortcuts.
	//e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(false, false, false)));
	//e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(false, false, true)));
	e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(false,
				true, false)));
	e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(false,
				true, true)));
	//e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(true, false, false)));
	//e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(true, false, true)));
	e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(true, true,
				false)));
	e_methods.push_back(multiwinner_stats(std::make_shared<qltd_pr>(true, true,
				true)));

	e_methods.push_back(multiwinner_stats(std::make_shared<old_qltd_pr>()));

	// Only enable this if we can handle the extremely large structure
	// it requires.
	e_methods.push_back(multiwinner_stats(std::make_shared<PSC>()));

	e_methods.push_back(multiwinner_stats(std::make_shared<STV>(BTR_NONE)));
	e_methods.push_back(multiwinner_stats(std::make_shared<STV>(BTR_PLUR)));
	e_methods.push_back(multiwinner_stats(std::make_shared<STV>(BTR_COND)));
	e_methods.push_back(multiwinner_stats(std::make_shared<MeekSTV>(true)));
	e_methods.push_back(multiwinner_stats(std::make_shared<MeekSTV>(false)));
	/*e_methods.push_back(multiwinner_stats(new QPQ(-1, false)));
	e_methods.push_back(multiwinner_stats(new QPQ(-1, true)));*/

	// Put on hold until we can do something about the huge memory
	// demands.
	e_methods.push_back(multiwinner_stats(std::make_shared<SchulzeSTV>()));

	// Not very good. Takes time, too.
	// And these were supposed to be "very good indeed"! What failed?
	// (Whatever I do, I get lousy results.)

	e_methods.push_back(multiwinner_stats(std::make_shared<birational>()));
	e_methods.push_back(multiwinner_stats(std::make_shared<log_penalty>()));
	e_methods.push_back(multiwinner_stats(std::make_shared<isoelastic>()));

	// It's possible to parameterize, but this way of passing parameters is
	// not very elegant.
	e_methods.push_back(multiwinner_stats(std::make_shared<isoelastic>(
				isoelastic_eval(-1))));
	e_methods.push_back(multiwinner_stats(std::make_shared<isoelastic>(
				isoelastic_eval(-0.5))));
	e_methods.push_back(multiwinner_stats(std::make_shared<isoelastic>(
				isoelastic_eval(0))));
	e_methods.push_back(multiwinner_stats(std::make_shared<isoelastic>(
				isoelastic_eval(2))));
	e_methods.push_back(multiwinner_stats(std::make_shared<log_penalty>(
				log_penalty_eval(1))));
	e_methods.push_back(multiwinner_stats(std::make_shared<log_penalty>(
				log_penalty_eval(10))));
	e_methods.push_back(multiwinner_stats(std::make_shared<log_penalty>(
				log_penalty_eval(1000))));

	// QPQ mass test
	// Disabled for now because QPQ needs a more extensive rework.
	/*
	double coeff;

	for (coeff = 0; coeff < 1.0; coeff += 0.1) {
		double rc;
		if (coeff == 0) {
			rc = 0.05;
		} else	{
			rc = coeff;
		}

		e_methods.push_back(multiwinner_stats(new QPQ(rc, false)));
	}

	for (coeff = 0; coeff < 1.0; coeff += 0.1) {
		double rc;
		if (coeff == 0) {
			rc = 0.05;
		} else	{
			rc = coeff;
		}

		e_methods.push_back(multiwinner_stats(new QPQ(rc, true)));
	}

	// QPQ hack
	multiwinner_stats qpqmetam("Best of QPQ(Meta, multiround)"), qpqmetas(
		"Best of QPQ(Meta, sequential)");*/


	/////////////////////////////////////////////////////////////////////////
	/// Done adding voting methods.
	/////////////////////////////////////////////////////////////////////////

	// I'm going to resize to one just to make debugging easier.
	/*e_methods.clear();
	e_methods.push_back(multiwinner_stats(std::make_shared<SchulzeSTV>()));*/

	int number = 0;

	// This should be turned into a proper multiwinner method. So should
	// "worst of n" and "best of n" for that matter.
	double sum_random = 0;

	election_t ballots;

	double sum_rbal = 0;

	double sum_worst = 0;
	double sum_best = 0;

	// Very quick and dirty majoritarian VSE data.
	std::map<std::string, double> method_utility;
	double random_utility = 0;
	double best_utility = 0;
	double cur_random_utility = 0, cur_best_utility = 0;
	size_t utility_count = 0;

	// DONE: Reproduce in secpr to see where we get different result.
	// IRV sounds way too low. It's not a glitch! It may be an effect of
	// the ballots assumptions or the council distributions, though..

	number = 0;
	int idx; // 55 to 56

	// 26 is example of Meek being worse than "ordinary" STV. Very strange.
	// 63 is another. Round 148 is an example where tiebreaking is needed
	// because at some point all remaining tie for quota.

	// 149
	// 156 crashed BTR-STV. No more.
	for (idx = 0; maxnum || run_forever; ++idx) {

		srandom(idx); // So we can replay in the case of bugs.
		srand(idx);
		srand48(idx);

		size_t num_voters = 5 + random() % 256;
		size_t num_candidates, council_size;
		int opinions = 1 + round(drand48() * 10);

		do {
			num_candidates = 5 + random() % 5;
		} while (num_candidates > num_voters);

		do {
			council_size = 2 + random() % 8;
		} while (council_size > num_candidates);

		cout << endl;
		cout << "=========================================" << endl;
		cout << " Round " << idx << endl;
		cout << " Voters: " << num_voters << "  Candidates: " << num_candidates <<
			"  Council size: " << council_size << "  Opinions: " << opinions << endl;

		++number;

		// Construct opinion profile

		vector<double> pop_opinion_profile(opinions, 0);

		// TODO: Find functional way of doing this
		for (int counter = 0; counter < opinions; ++counter) {
			pop_opinion_profile[counter] = drand48();
		}

		vector<vector<bool> > population_profiles;

		while (population_profiles.size() < num_voters)
			population_profiles.push_back(generate_opinion_profile(
					pop_opinion_profile));

		// DEBUG.
		cout << "Candidate profiles:" << endl;
		print_profiles(population_profiles, 0, num_candidates);

		// Our binary profiles per voter may not sum up exactly to the
		// percentage support for each profile in society. Fix this here.
		pop_opinion_profile = sum_opinion_profile(population_profiles);

		cout << "Population opinion profile: ";
		copy(pop_opinion_profile.begin(), pop_opinion_profile.end(),
			ostream_iterator<double>(cout, " "));
		cout << endl;

		// To test the memory leak, perhaps put this outside the loop
		// and fix num_candidates and num_voters? The results would be
		// nonsensical, but that's not what we're trying to fix anyhow.
		ballots.clear();
		ballots = construct_ballots(population_profiles,
				num_candidates, 0.001);

		// Calculate majoritarian VSE.
		std::vector<double> utilities = get_utilities(ballots,
				num_candidates);
		cur_best_utility = get_optimal_utility(utilities,
				council_size);
		cur_random_utility = get_random_utilities(utilities);

		best_utility += cur_best_utility;
		random_utility += cur_random_utility;
		++utility_count;

		std::cout << "VSE: random: " << cur_random_utility << ", best: " <<
			cur_best_utility << std::endl;
		std::cout << "Total: random: " << random_utility/utility_count <<
			", best: " << best_utility/utility_count << "\n";

		// Get the disproportionality of random councils, which is needed
		// to anchor the proportionality of random candidate for the VSE-
		// like disproportionality measure.

		// We could also run this through for_each_combination. TODO later.

		double random_disprop = 0;
		size_t max_disprop_iters = 1000;

		for (size_t i = 0; i < max_disprop_iters; ++i) {
			std::list<int> random_draw = random_council(num_candidates,
					council_size);

			random_disprop += get_error(random_draw,
					num_candidates, council_size,
					population_profiles,
					pop_opinion_profile);
		}

		// The stuff below should use proper multiwinner_stats objects.
		// TODO!!!

		random_disprop /= (double)max_disprop_iters;
		std::cout << "Random candidate (raw): " << random_disprop << "\n";

		double worst, average, best;
		get_limits(num_candidates, council_size, population_profiles,
			pop_opinion_profile, 75000, worst, average,
			best);

		sum_worst += worst;
		sum_best += best;

		cout << "Worst: " << worst << ", Average: " << average <<
			", Best: " << best << endl;

		sum_random += random_disprop;

		cout << display_stats(0, number, random_disprop,
				0, "-- Random candidates --") << endl;

		// Create ballots consistent with the population's preferences.
		// (Yuck, this is ugly...)

		// Not good
		double rbal_value = random_ballot(ballots, num_candidates,
				council_size, num_candidates / 4,
				population_profiles, pop_opinion_profile,
				10000);

		sum_rbal += rbal_value;

		double random_ballot_vse = (sum_rbal - sum_random) /
			(sum_best - sum_random);
		double this_round_rb_vse = (rbal_value - random_disprop) /
			(best - random_disprop);

		cout << display_stats(random_ballot_vse, 1, rbal_value,
				this_round_rb_vse, "-- Random ballot (0.25) --") << endl;

		double error;

		std::cout << "\n";
		std::cout << s_padded("Prop.", 8) << " " << s_padded("VSE",
				8) << " Method\n";
		for (size_t counter = 0; counter < e_methods.size(); ++counter) {
			if (e_methods[counter].method() == NULL) {
				continue;
			}
			// Hack for extremely slow methods. Note that this
			// changes the sampling for those methods!
			// Should be 10; this is just a hack for Schulze,
			// and ideally we should have "permissible" within
			// the method itself, to calculate for each method
			// whether we can reasonably find the winners.
			if (!e_methods[counter].method()->polytime() && council_size > 15) {
				continue;
			}

			std::list<int> council = e_methods[counter].method()->
				get_council(council_size,
					num_candidates,	ballots);

			error = get_error(council,
					num_candidates, council_size,
					population_profiles,
					pop_opinion_profile);

			e_methods[counter].add_result(rbal_value, error, best);

			// VSE update

			double this_method_utility = get_council_utility(utilities,
					council);

			if (method_utility.find(e_methods[counter].get_name()) ==
				method_utility.end()) {
				method_utility[e_methods[counter].get_name()] = this_method_utility;
			} else {
				method_utility[e_methods[counter].get_name()] += this_method_utility;
			}

			double total_utility = method_utility[e_methods[counter].get_name()];
			double VSE = (total_utility - random_utility) / (best_utility -
					random_utility);

			// Print it all out.

			cout << e_methods[counter].display_stats(VSE) << endl;
		}

		// Meta
		/*set_best(qpqmetas, best, worst, e_methods, "QPQ", "sequential");
		cout << qpqmetas.display_stats() << endl;
		set_best(qpqmetam, best, worst, e_methods, "QPQ", "multiround");
		cout << qpqmetam.display_stats() << endl;*/
	}

	/*cout << "Borda: " << error << ", norm: " << renorm(best, worst, error,
			0.0, 1.0) << endl;

	error = get_error(majoritarian_council(council_size, num_candidates,
				ballots, &hare), population_profiles,
			pop_opinion_profile);

	cout << "Other: " << error << ", norm: " << renorm(best, worst, error,
			0.0, 1.0) << endl;*/
}
