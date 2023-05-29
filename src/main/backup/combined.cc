// All in one! The idea here is to build every possible simple method
// combination we can think of, then either dump it all so the user can specify
// which methods interest him, or intersect it with such a list winnowed down
// by the user. A more sophisticated version might try to parse the actual names
// so that we don't have to init every possible combination of methods
// beforehand. (BLUESKY do that, which would thus permit things like LE/LE/LE/
// (Smith,Plurality).)

#include <assert.h>
#include <values.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <list>

// General includes - tools and ballot structures.

#include "../tools/ballot_tools.h"
#include "../tools/tools.h"
#include "../ballots.h"

// Generators go here.

#include "../generator/all.h"

// Single-winner methods, including metamethods.

// DONE: Meta-header that includes all of these.
// TODO? Perhaps move the "factory" itself into that, too.
#include "../singlewinner/all.h"

// More later. POC for now. Perhaps also do something with the fact that C++
// doesn't have garbage collection -- i.e. clean up after ourselves.

// Headers for the Bayesian regret mode:
#include "../random/random.h"
#include "../stats/stats.h"

#include "../modes/breg.h"

// Headers for the Yee mode:
#include "../images/color/color.h"
#include <openssl/sha.h>

list<pairwise_method *> get_pairwise_methods(
	const list<pairwise_type> & types) {

	// For each type, and for each pairwise method, dump that combination
	// to the output. Possible feature request: have the method determine
	// if it supports the type in question (e.g. types that can give < 0
	// wrt stuff like Keener).

	list<pairwise_method *> out;

	for (list<pairwise_type>::const_iterator pos = types.begin(); pos !=
		types.end(); ++pos) {
		out.push_back(new dquick(*pos));
		out.push_back(new kemeny(*pos));
		out.push_back(new maxmin(*pos));
		out.push_back(new schulze(*pos));

		// Reasonable convergence defaults.
		out.push_back(new sinkhorn(*pos, 0.01, true));
		out.push_back(new sinkhorn(*pos, 0.01, false));
		out.push_back(new hits(*pos, 0.001));
		out.push_back(new odm_atan(*pos, 0.001));
		out.push_back(new odm(*pos, 0.001));

		out.push_back(new ext_minmax(*pos, false));
		out.push_back(new ext_minmax(*pos, true));
		out.push_back(new ord_minmax(*pos));
	}

	// Doesn't matter what these are set to. There should be a test for that
	// so that we can just do an if.

	// Defined elsewhere...
	//out.push_back(new copeland(CM_WV));

	out.push_back(new copeland(CM_WV, 2, 2, 1));
	out.push_back(new copeland(CM_WV, 2, 1, 0));
	out.push_back(new randpair(CM_WV));

	return (out);
}

// Positional methods.

list<positional *> get_positional_methods() {
	// Put that elsewhere?
	list<positional_type> types;
	for (int p = PT_FIRST; p <= PT_LAST; ++p) {
		types.push_back(positional_type(p));
	}

	list<positional *> out;

	for (list<positional_type>::const_iterator pos = types.begin(); pos !=
		types.end(); ++pos) {
		out.push_back(new plurality(*pos));
		out.push_back(new borda(*pos));
		out.push_back(new antiplurality(*pos));
		out.push_back(new for_and_against(*pos));
		out.push_back(new nauru(*pos));
		out.push_back(new heismantrophy(*pos));
		out.push_back(new baseballmvp(*pos));
		out.push_back(new eurovision(*pos));
		out.push_back(new dabagh(*pos));
		out.push_back(new nrem(*pos));
		out.push_back(new worstpos(*pos));
		out.push_back(new worstborda(*pos));
	}

	return (out);
}

list<pairwise_method *> get_sets() {

	list<pairwise_method *> out;

	out.push_back(new condorcet_set());
	out.push_back(new mdd_set(true));
	out.push_back(new mdd_set(false));
	out.push_back(new partition_set(false));

	out.push_back(new cdtt_set());
	out.push_back(new cgtt_set());
	out.push_back(new landau_set());
	out.push_back(new schwartz_set());
	out.push_back(new sdom_set());
	out.push_back(new smith_set());

	out.push_back(new copeland(CM_WV)); // Nudge nudge.

	return (out);
}

template <typename T, typename Q> list<election_method *> expand_meta(
	const list<T *> & base_methods, const list<Q *> & sets,
	bool is_positional) {

	list<election_method *> kombinat;
	typename list<Q *>::const_iterator spos;

	for (typename list<T *>::const_iterator pos = base_methods.begin();
		pos != base_methods.end(); ++pos) {
		for (spos = sets.begin(); spos != sets.end(); ++spos) {
			if ((*pos)->name() == (*spos)->name()) {
				continue;
			}

			kombinat.push_back(new comma(*pos, *spos));
			// Use indiscriminately at your own risk! I'm not
			// trusting this wholly until I can do hopefuls
			// transparently.
			if (is_positional) {
				kombinat.push_back(new slash(*pos, *spos));
			}
		}
		// These are therefore constrained to positional
		// methods for the time being. I think there may be bugs with
		// hopefuls for some of the advanced methods. I'm therefore
		// not doing anything with them, limiting LE and slash to
		if (is_positional) {
			kombinat.push_back(new loser_elimination(*pos, true,
					true));
			kombinat.push_back(new loser_elimination(*pos, false,
					true));
		}
	}

	return (kombinat);
}


list<election_method *> get_singlewinner_methods() {

	list<election_method *> toRet;

	list<pairwise_method *> pairwise = get_pairwise_methods(
			pairwise_producer().provide_all_strategies());

	copy(pairwise.begin(), pairwise.end(), back_inserter(toRet));

	list<positional *> positional_methods = get_positional_methods();
	list<pairwise_method *> pairwise_sets = get_sets();

	// We have to do it in this clumsy manner because of possible bugs in
	// handling methods with some candidates excluded.
	list<election_method *> posnl_expanded = expand_meta(positional_methods,
			pairwise_sets, true);

	// Now add other methods here...
	copy(pairwise_sets.begin(), pairwise_sets.end(), back_inserter(toRet));

	// Gradual Condorcet-Borda with different bases, not just Condorcet.
	// The completion method doesn't really matter. Also, MDD* doesn't
	// really work here, and sets that can't handle negatives shouldn't
	// be applied to it.
	for (list<pairwise_method *>::const_iterator basis = pairwise_sets.
			begin(); basis != pairwise_sets.end(); ++basis) {
		toRet.push_back(new gradual_cond_borda(*basis, false, GF_BOTH));
		toRet.push_back(new gradual_cond_borda(*basis, true, GF_BOTH));
	}

	toRet.push_back(new young(true, true));
	toRet.push_back(new young(false, true));
	toRet.push_back(new random_ballot());
	toRet.push_back(new random_candidate());
	toRet.push_back(new cardinal_ratings(0, 10, false));
	toRet.push_back(new cardinal_ratings(0, 10, true));
	toRet.push_back(new mode_ratings());
	toRet.push_back(new vi_median_ratings(10, false, false));
	toRet.push_back(new vi_median_ratings(10, false, true));
	toRet.push_back(new vi_median_ratings(10, true, false));
	toRet.push_back(new vi_median_ratings(10, true, true));

	// Then expand:
	list<election_method *> expanded = expand_meta(toRet, pairwise_sets,
			false);

	// and

	copy(positional_methods.begin(), positional_methods.end(),
		back_inserter(toRet));
	copy(posnl_expanded.begin(), posnl_expanded.end(),
		back_inserter(toRet));
	copy(expanded.begin(), expanded.end(), back_inserter(toRet));

	// Done!
	return (toRet);
}

void do_regret(list<election_method *> & methods) {

	// Kludge together something with Bayesian regret here. DONE: Move over
	// to modes.

	list<pure_ballot_generator *> generators;
	generators.push_back(new spatial_generator(true, false));
	generators.push_back(new impartial(true, false));

	int maxiters = 40000;
	int min_candidates = 4, max_candidates = 20;
	int min_voters = 3, max_voters = 200;

	int report_frequency = 200;

	list<const election_method *> rtmethods; // What a kludge.
	copy(methods.begin(), methods.end(), back_inserter(rtmethods));

	bayesian_regret br(maxiters, min_candidates, max_candidates,
		min_voters, max_voters, false, MS_INTRAROUND,
		generators, rtmethods);

	assert(br.init());

	string status;

	rng randomizer(10); // <- or time

	int counter = 0;

	cache_map cache;

	do {
		cache.clear();
		status = br.do_round(true, true, randomizer, cache);
		cout << status << endl;

		if (++counter % report_frequency == (report_frequency - 1)) {
			vector<string> report = br.provide_status();
			copy(report.begin(), report.end(),
				ostream_iterator<string>(cout, "\n"));
		}
	} while (status != "");
}

// am_ac_winners is short for "all methods, all candidates, winners".

// use_autopilot enables an IEVS-style "autopilot". This starts at a relatively
// low number of voters, scaling up until the autopilot_history last all have
// the same number of winners or we exceed max_num_voters. Doing this with
// cache is kinda tricky. If use_autopilot is false, we always use
// max_num_voters.
// (To get the maximum possible out of the autopilot history if we never
//  converge, we feed the output orderings to a meta-method and pick a winner
//  accordingly. Currently, this meta-method is Schulze; we can't use CR since
//  not all methods provide social scores, some only give social orderings.
//  BLUESKY: move is_rated into ordering so we *can*.)

// Returns -1 on error, otherwise the total number of candidates' worth we
// checked.

// TODO: Give this one access to the winners. After having calculated who won,
// check if the square around us (except those that have no -1 and at least one
// +1, because we don't know those yet) agree. If not, check further. That
// should diminish salt-and-pepper noise. BLUESKY: Render using PNG
// interpolation order to maximize effect.

int check_pixel(int x, int y, int xsize, int ysize, const vector<const
	election_method *> & methods, spatial_generator & ballotgen,
	vector<vector<vector<vector<bool > > > > & am_ac_winners,
	int min_num_voters, int max_num_voters, bool use_autopilot,
	double autopilot_factor, int autopilot_history, cache_map &
	cache, rng & randomizer) {

	size_t num_cands = am_ac_winners[0].size(),
		   num_methods = am_ac_winners.size();

	vector<list<ballot_group> > autopilot_am(num_methods);
	vector<int> num_identical(num_methods, 0);
	list<ballot_group> ballots;

	// Arrange these so (0,0) is at the bottom left?
	vector<double> relative(2);
	relative[0] = x / (double) xsize;
	relative[1] = y / (double) ysize;

	if (!ballotgen.set_mean(relative)) {
		return (-1);
	}

	double cur_num_voters = min_num_voters;

	if (!use_autopilot) {
		cur_num_voters = max_num_voters;
	}

	size_t method, bottom_line = 0;

	ordering out, rank_out;

	int cleared = 0;

	ordering_tools otools;

	while ((int)round(cur_num_voters) <= max_num_voters &&
		cleared < num_methods) {

		// Sample the voter distribution at our pixel.
		ballots = ballotgen.generate_ballots(round(cur_num_voters),
				num_cands, randomizer);

		cache.clear();

		for (method = 0; method < num_methods && cleared < num_methods;
			++method) {
			// If we've already got enough data to say who this
			// method elects, ignore it.
			if (num_identical[method] >= autopilot_history) {
				continue;
			}

			out = methods[method]->elect(ballots, num_cands, cache,
					true);

			rank_out = ordering_tools().scrub_scores(out);

			// If it's the same, add to the count, otherwise reset
			// it. Also check if we've passed the threshold; if so,
			// add to cleared with hope of getting out early.
			if (autopilot_am[method].empty() || otools.winner_only(
					rank_out) ==
				otools.winner_only(autopilot_am[method].rbegin()->
					contents)) {
				++num_identical[method];
				if (num_identical[method] >= autopilot_history) {
					++cleared;
				}
			} else	{
				num_identical[method] = 0;
			}

			// In either case, add it to the list so we can use it
			// to find out who won, later.
			autopilot_am[method].push_back(ballot_group(
					round(cur_num_voters), rank_out,
					true, false));

			bottom_line += cur_num_voters;
		}

		cur_num_voters *= max(1.01, autopilot_factor);
	}

	// Now simply paint the appropriate pixels depending on who won.

	for (method = 0; method < num_methods; ++method) {

		ordering meta = schulze(CM_WV).elect(autopilot_am[method],
				num_cands, *(cache_map *)(NULL), true);

		// For all the winners, paint his boolean true.

		for (ordering::const_iterator opos = meta.begin(); opos !=
			meta.end() && opos->get_score() ==
			meta.begin()->get_score(); ++opos)
			am_ac_winners[method][opos->get_candidate_num()][x][y]
				= true;
	}

	return (bottom_line);
}

bool draw_pictures(string prefix, const vector<vector<vector<bool > > > &
	ac_winners, vector<vector<double> > & cand_colors,
	vector<vector<double> > & cand_locations,
	bool draw_binaries, bool ignore_errors) {

	// First draw all the binary pictures (if so requested).
	// TODO: PNG yada yada. Don't have time for it now.

	size_t numcands = cand_colors.size(), counter, x, y,
		   xsize = ac_winners[0].size(), ysize = ac_winners[0][0].size();

	bool okay_so_far = true;
	string outfn;

	for (counter = 0; counter < numcands && draw_binaries; ++counter) {
		outfn = prefix + "_bin_c" + dtos(counter) + "won.pgm";
		ofstream outfile(outfn.c_str());

		if (!outfile) {
			okay_so_far = false;
			if (!ignore_errors && !okay_so_far) {
				cerr << "Couldn't open binary picture file "
					<< outfn << " for writing." << endl;
				return (false);
			}

			continue;
		}

		// Header!
		outfile << "P5 " << endl << xsize << " " << ysize << endl
			<< 255 << endl;

		// Dump the boolean.
		for (y = 0; y < ysize; ++y)
			for (x = 0; x < xsize; ++x) {
				if (ac_winners[counter][x][y]) {
					outfile << (char) 255;
				} else	{
					outfile << (char) 0;
				}
			}

		outfile.close(); // And all done.
	}

	// Build the color picture.

	outfn = prefix + "_yee.ppm";
	ofstream color_pic(outfn.c_str());

	if (!color_pic) {
		// Couldn't open that file.
		cerr << "Couldn't open color picture file " << outfn <<
			" for writing." << endl;
		// Since we've got nothing more to do, we can just return false
		// no matter what here.
		return (false);
	}

	// Write the header.
	color_pic << "P6 " << endl << xsize << " " << ysize << endl << 255
		<< endl;

	vector<double> adj_coords(2);

	for (y = 0; y < ysize; ++y) {
		adj_coords[1] = y / (double)ysize;
		for (x = 0; x < xsize; ++x) {
			adj_coords[0] = x / (double)xsize;
			// If there's a tie, we output the mean color. If we
			// go to LAB at some later point, this may become more
			// complex.
			vector<double> prosp_RGB(3);
			int num_winners = 0;

			// We set these if any of the candidates have their
			// "home circle" around this pixel. It means we should
			// not draw anything but that candidate's home circle
			// at this point.
			int outer_border_of = -1, inner_border_of = -1;
			bool is_home = false;
			int copier;

			for (int cand = 0; cand < numcands && !is_home;
				++cand) {
				double dist = euc_distance(2.0, adj_coords,
						cand_locations[cand]);

				if (dist < 2.5/(double)xsize +
					2.5/(double)ysize) {
					outer_border_of = cand;
					is_home = true;
				}

				if (dist < 1.5/(double)xsize +
					1.5/(double)ysize) {
					inner_border_of = cand;
				}

				if (!ac_winners[cand][x][y] || is_home) {
					continue;
				}

				for (copier = 0; copier < 3; ++copier)
					prosp_RGB[copier] += cand_colors[cand]
						[copier] * 0.96;
				++num_winners;
			}

			// Check if it's the home of someone. If so, color
			// appropriately (black border if outer, color if
			// inner). If not, go ahead and plunk down our pixel.

			if (is_home) {
				if (inner_border_of != -1)
					prosp_RGB = cand_colors[
							inner_border_of];
				else
					for (copier = 0; copier < 3; ++copier) {
						prosp_RGB[copier] = 0;
					}

				num_winners = 1; // all of that OVER 1. -KA :p
			}

			// 255.4999 to make maximum use of the color space. The
			// greatest value we can output is 255, and that should
			// fit 1 exactly, so it should be just below the point
			// where round will round to 256, hence 255.499...
			for (copier = 0; copier < 3; ++copier)
				color_pic << (char)round(255.4999 *
						prosp_RGB[copier]/
						(double)num_winners);
		}
	}

	color_pic.close();
	return (okay_so_far);

}

vector<vector<double> > get_candidate_colors(int numcands, bool debug) {

	if (numcands <= 0) {
		return (vector<vector<double> >());
	}

	vector<vector<double> > candidate_RGB(numcands);

	// The candidates are each given colors at hues spaced equally from
	// each other, and with full saturation and value. If you want
	// IEVS-style sphere packing, feel free to alter (call a class), but
	// it might be better in LAB color space -- if I could get LAB to work.

	vector<double> HSV(3, 1);
	HSV[0] = 0;

	color_conv converter;

	for (int cand = 0; cand < numcands; ++cand) {
		candidate_RGB[cand] = converter.convert(HSV, CS_HSV, CS_RGB);

		if (debug) {
			cout << "RGB values for " << cand << ": ";
			copy(candidate_RGB[cand].begin(),
				candidate_RGB[cand].end(),
				ostream_iterator<double>(cout, "\t"));

			cout << endl;
		}

		// It must be +1 because the Hue aspect is circular, leaving
		// just as much space between the last candidate's hue and the
		// first, as between the first and second's.

		HSV[0] += 1 / (double)(numcands+1);
	}

	return (candidate_RGB);
}

string get_sha_code(const election_method & in, int bytes) {

	string method_name = in.name();

	unsigned char * input = (unsigned char *)calloc(method_name.size() + 1,
			sizeof(char));

	if (input == NULL) {
		return ("");
	}

	unsigned char * output = (unsigned char *)calloc(256/8, sizeof(char));

	if (output == NULL) {
		free(input);
		return ("");
	}

	copy(method_name.begin(), method_name.end(), input);

	if (SHA256(input, method_name.size(), output) == NULL) {
		free(input);
		free(output);
		return ("");
	}

	// For however many bytes we want, use itos_hex to construct a string.
	string outstr;
	for (int counter = 0; counter < min(bytes, 256/8); ++counter) {
		outstr += itos_hex(output[counter], 2);
	}

	// Free input and output and return string.
	free(input);
	free(output);
	return (outstr);
}


int main() {
	// DONE: Use first 32 bits or so of MD5sum in hex as prefix. Or SHA,
	// since we're already using it.

	list<election_method *> methods = get_singlewinner_methods();
	list<election_method *>::const_iterator pos;

	for (pos = methods.begin(); pos != methods.end(); ++pos) {
		cout << (*pos)->name() << "\t" << get_sha_code(**pos, 5) << endl;
	}

	cout << "Something new!" << endl;

	// Hack something resembling Yee here.
	// For the first method (don't want to kill ourselves in one stroke)
	// for each x,y, make a ballot set from a Gaussian centered at that
	// position with specified mean and sigma, and with given candidate
	// positions.

	// Then do something with libpng and colors and whatever afterwards.

	vector<const election_method *> test;
	test.push_back(new vi_median_ratings(10, true, true));
	test.push_back(new plurality(PT_WHOLE));
	test.push_back(new loser_elimination(new plurality(PT_WHOLE), false,
			true));
	test.push_back(*methods.begin());
	rng randomizer(11); // or time.

	int xsize = 400, ysize = 400;
	int numcands = 4, numvoters = 1000;
	double sigma = 0.3;

	spatial_generator uniform(true, false); // For setting candidate positions.
	uniform.set_params(2, true); // Two dimenshuns.

	// Set candidate positions.
	assert(uniform.fix_candidate_positions(numcands, randomizer));

	// Get those positions and transplant into the Gaussian.
	gaussian_generator gaussian(true, false);
	assert(gaussian.fix_candidate_positions(numcands, uniform.
			get_fixed_candidate_pos()));
	assert(gaussian.set_sigma(sigma));

	// For each point, generate associated ballots, determine who wins,
	// and plot that point.
	vector<vector<vector<vector<bool> > > > winner(test.size(),
		vector<vector<vector<bool> > >(numcands,
			vector<vector<bool> >(xsize,
				vector<bool>(ysize, false))));

	vector<double> coords(2);

	cache_map cache;
	int x, y;

	int in_all = 0;

	double start_new = get_abs_time();

	for (x = 0; x < xsize; ++x) {
		cout << "Yee test: x = " << x << " of " << xsize << "\t" <<
			flush;
		int grand_sum = 0;
		for (y = 0; y < ysize; ++y) {
			int contrib = check_pixel(x, y, xsize, ysize, test,
					gaussian, winner, 24, numvoters, true,
					1.3, 4, cache, randomizer);
			if (contrib == -1) {
				cout << "Error at x " << x << ", y " << y <<
					endl;
				return (-1);
			}
			grand_sum += contrib;
		}
		in_all += grand_sum;
		cout << "for " << grand_sum << endl;
	}

	cout << in_all << " in all. " << endl;
	double start_old = get_abs_time();
	cout << "Time: " << start_old - start_new << endl;

	// Determine colors for each candidate.

	vector<vector<double> > candidate_RGB = get_candidate_colors(numcands,
			false);

	vector<vector<double> > cand_locs = uniform.get_fixed_candidate_pos();

	for (int method_no = 0; method_no < test.size(); ++method_no) {
		string code = get_sha_code(*test[method_no], 5);

		cout << "Yee: " << test[method_no]->name() << " has code "
			<< code << ". Drawing... " << flush;
		if (draw_pictures("k2_" + code, winner[method_no],
				candidate_RGB, cand_locs, true,
				false)) {
			cout << " OK. " << endl;
		} else {
			cout << " error!" << endl;
		}
	}
}
