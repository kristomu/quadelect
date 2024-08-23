
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

#include "multiwinner/psc.h"
#include "multiwinner/qbuck.h"
#include "multiwinner/meek_stv.h"
#include "multiwinner/shuntsstv.h"
#include "multiwinner/compat_qbuck.h"
#include "multiwinner/hard_card.h"
#include "multiwinner/dhwl.h"
#include "multiwinner/stv.h"
//#include "multiwinner/qpq.cc"

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <ext/numeric>
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

list<int> random_council(int num_candidates, int council_size) {

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
list<int> random_ballot(int num_ballots,
	const election_t & ballots,
	int num_candidates, size_t council_size, size_t accept_number) {

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
	vector<int> accepted(requested, 0);
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
		assert(accepted[counter] >= 0);
		toRet.push_back(accepted[counter]);
	}

	return (toRet);
}


double get_error(const list<int> & council, int num_candidates,
	int council_size,
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

void print_sstv_pref(const election_t & f, int council_size, int
	num_candidates) {

	cerr << "M " << council_size << endl;
	cerr << "C " << num_candidates << endl;
	cerr << "N " << f.size() << endl;
	cerr << "F 2" << endl; // format type

	cerr << endl << "BEGIN" << endl;

	int voter = 1;

	for (election_t::const_iterator pos = f.begin(); pos != f.end();
		++pos) {
		cerr << voter++ << " ";
		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos) {
			cerr << (char)('A' + opos->get_candidate_num()) << " ";
		}
		cerr << endl;
	}
	cerr << "END" << endl;
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
	if (argc < 2) {
		cerr << "Specfiy max match number." << endl;
		return (-1);
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

	// TODO: FIX
	/*condorcet.push_back(std::make_shared<minmax>(CM_WV));
	condorcet.push_back(std::make_shared<minmax>(CM_MARGINS));*/
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

	for (counter = 1; counter < condorcet.size(); ++counter) {
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
	//e_methods.push_back(multiwinner_stats(std::make_shared<PSC>()));

	e_methods.push_back(multiwinner_stats(std::make_shared<STV>(BTR_NONE)));
	e_methods.push_back(multiwinner_stats(std::make_shared<STV>(BTR_PLUR)));
	e_methods.push_back(multiwinner_stats(std::make_shared<STV>(BTR_COND)));
	e_methods.push_back(multiwinner_stats(std::make_shared<MeekSTV>(true)));
	e_methods.push_back(multiwinner_stats(std::make_shared<MeekSTV>(false)));
	/*e_methods.push_back(multiwinner_stats(new QPQ(-1, false)));
	e_methods.push_back(multiwinner_stats(new QPQ(-1, true)));*/

	// Put on hold until we can do something about the huge memory
	// demands.
	//e_methods.push_back(multiwinner_stats(new SchulzeSTV()));

	// Not very good. Takes time, too.
	// And these were supposed to be "very good indeed"! What failed?
	// (Whatever I do, I get lousy results.)

	/*e_methods.push_back(multiwinner_stats(new hardcard(HC_BIRATIONAL)));
	e_methods.push_back(multiwinner_stats(new hardcard(HC_LPV)));*/

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

	int number = 0;

	// This should be turned into a proper multiwinner method. So should
	// "worst of n" and "best of n" for that matter.
	double sum_random = 0;

	election_t ballots;

	double sum_rbal = 0;

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
	for (idx = 0; idx < maxnum; ++idx) {

		srandom(idx); // So we can replay in the case of bugs.
		srand(idx);
		srand48(idx);

		/*int num_voters = 512 + round((drand48() * 512));
		int num_candidates = 31 + round(drand48() * 100);
		int council_size = 3 * round((drand48() + 1) * 5);
		int opinions = 1 + round(drand48() * 10);*/

		size_t num_voters =  5 + pow(5, 1 + drand48() * 4.72);
		size_t num_candidates = min(max(10.0, num_voters/10.0), pow(5,
					1 + drand48() * 3.29));
		size_t council_size = max(1.0, min(max(1.0, num_candidates/2.0),
					400-((double)(1 + random() % 20)*(1+random()%20))));
		// KLUDGE to test PSC-CLE. I need to find a way of doing this in
		// polyspace.
		//while (num_candidates > 80) { num_candidates >>= 1; council_size >>= 1; }

		int opinions = 1 + round(drand48() * 10);

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

		// Readjust to handle quantization errors wrt the entire population.
		// We could do this in a more sophisticated way by randomizing according
		// to the first n issues, then setting 0..bias/total to true and the
		// rest to false, before randomizing again, and so on. Later.
		pop_opinion_profile = sum_opinion_profile(population_profiles);

		cout << "Population opinion profile: ";
		copy(pop_opinion_profile.begin(), pop_opinion_profile.end(),
			ostream_iterator<double>(cout, " "));
		cout << endl;

		double worst, average, best, norm_average;
		get_limits(num_candidates, council_size, population_profiles,
			pop_opinion_profile, 75000, worst, average,
			best);

		cout << "Worst: " << worst << ", Average: " << average <<
			", Best: " << best << endl;

		norm_average = renorm(best, worst, average, 0.0, 1.0);

		sum_random += norm_average;

		cout << display_stats(sum_random, number,
				average, norm_average, "-- Random candidates --") << endl;

		// To test the memory leak, perhaps put this outside the loop
		// and fix num_candidates and num_voters? The results would be
		// nonsensical, but that's not what we're trying to fix anyhow.
		ballots.clear();
		ballots = construct_ballots(population_profiles,
				num_candidates, 0.001);
		//print_sstv_pref(ballots, council_size, num_candidates);

		// Not good
		double rbal_value = random_ballot(ballots, num_candidates,
				council_size, num_candidates / 4,
				population_profiles, pop_opinion_profile,
				10000);

		double norm_rbal = renorm(best, worst, rbal_value, 0.0, 1.0);

		sum_rbal += norm_rbal;

		cout << display_stats(sum_rbal, number, rbal_value,
				norm_rbal, "-- Random ballot (0.25) --") << endl;

		/* Testing something
		// TODO: Make it possible to alter weights without having to
		// copy the entire ballot list.
		double rvp_value = get_error(voter_reweighted_council(
					council_size, num_candidates,
					ballots, (positional *)e_methods[1].
					method), council_size,
				population_profiles, pop_opinion_profile);

		double norm_rvp = renorm(best, worst, rvp_value, 0.0, 1.0);

		sum_rvp += norm_rvp;

		cout << display_stats(sum_rvp, number, rvp_value, norm_rvp,
				"(Reweighted Borda)") << endl;*/

		double error;
		//double norm_error;

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
			error = get_error(e_methods[counter].method()->
					get_council(council_size,
						num_candidates,	ballots),
					num_candidates, council_size,
					population_profiles,
					pop_opinion_profile);
			//norm_error = renorm(best, worst, error, 0.0, 1.0);

//			cout << "\t" << e_methods[counter].name << ": " << error << ", norm: " << norm_error << "\t";
			e_methods[counter].add_result(best, error, worst);
			/*e_methods[counter].sum_scores += error;
			e_methods[counter].sum_normalized_scores += norm_error;

			cout << display_stats(e_methods[counter], number, error, norm_error) << endl;*/
			cout << e_methods[counter].display_stats() << endl;

			//		cout << "avg norm.: " << e_methods[counter].sum_normalized_scores / (double)number << "\tunnorm: " << e_methods[counter].sum_scores / (double)number << endl;
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
