// Mode for drawing Yee diagrams. Yee diagrams consist of plotting, for each
// pixel in a picture (translated to a point in [0...1]^2), who wins if the
// voters are modeled as a normal distribution centered on that point, and each
// voter prefers a candidate nearby to a candidate further away.

// Currently only supports writing to PGM and PPM formats. PNG may come later.

#include "yee.h"
#include "../images/color/color.h"
#include "../singlewinner/pairwise/simple_methods.h"

#include <fstream>

// The methods here are a bit out of order because this used to be in
// combined.cc. Use search to find the functions you want.

//////

// am_ac_winners is short for "all methods, all candidates, winners".

// use_autopilot_in enables an IEVS-style "autopilot". This starts at a relatively
// low number of voters, scaling up until the autopilot_history_in last all have
// the same number of winners or we exceed max_num_voters_in. Doing this with
// cache is kinda tricky. If use_autopilot_in is false, we always use 
// max_num_voters_in.
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

long long yee::check_pixel(int x, int y, int xsize_in, int ysize_in, 
		const vector<const election_method *> & methods, 
		spatial_generator & ballotgen,
		vector<vector<vector<vector<bool > > > > & am_ac_winners,
		int min_num_voters_in, int max_num_voters_in, 
		bool use_autopilot_in, double autopilot_factor_in, 
		int autopilot_history_in, cache_map * cache, 
		rng & randomizer) const {

	size_t num_cands = am_ac_winners[0].size(), 
	       num_methods = am_ac_winners.size();

	vector<list<ballot_group> > autopilot_am(num_methods);
	vector<int> num_identical(num_methods, 0);
	list<ballot_group> ballots;

	// Arrange these so (0,0) is at the bottom left?
	vector<double> relative(2);
	relative[0] = x / (double) xsize_in;
	relative[1] = y / (double) ysize_in;

	if (!ballotgen.set_center(relative))
		return(-1);

	double cur_num_voters = min_num_voters_in;

	if (!use_autopilot_in)
		cur_num_voters = max_num_voters_in;

	size_t method;
	long long bottom_line = 0;

	ordering out, rank_out;

	size_t cleared = 0;

	ordering_tools otools;

	double def_autopilot_factor = 1.01;

	while ((int)round(cur_num_voters) <= max_num_voters_in && 
			cleared < num_methods) {

		// Sample the voter distribution at our pixel.
		// Note: generate_ballots is quite expensive. Consider the value
		// of using autopilot...
		ballots = ballotgen.generate_ballots(round(cur_num_voters), 
				num_cands, randomizer);

		cache->clear();

		for (method = 0; method < num_methods && cleared < num_methods;
				++method) {
			// If we've already got enough data to say who this 
			// method elects, ignore it.
			if (num_identical[method] >= autopilot_history_in)
				continue;

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
				if (num_identical[method] >= autopilot_history_in)
					++cleared;
			}
			else	num_identical[method] = 0;

			// In either case, add it to the list so we can use it
			// to find out who won, later.
			autopilot_am[method].push_back(ballot_group(
						round(cur_num_voters), rank_out,
						true, false));

			bottom_line += cur_num_voters;
		}

		if (cur_num_voters != max_num_voters)
			cur_num_voters = min((double)max_num_voters, 
					cur_num_voters * max(def_autopilot_factor,
					autopilot_factor_in));
		else
			cur_num_voters *= max(def_autopilot_factor, 
				autopilot_factor_in);
	}

	// Now simply paint the appropriate pixels depending on who won.

	for (method = 0; method < num_methods; ++method) {

		// TODO: Do something else when we're dealing with probabilistic
		// methods. "n tries of Random Pair run through Schulze" is not
		// the same thing as Random Pair!
		ordering meta = schulze(CM_WV).elect(autopilot_am[method],
				num_cands, true);

		// For all the winners, paint his boolean true.

		for (ordering::const_iterator opos = meta.begin(); opos !=
				meta.end() && opos->get_score() == 
				meta.begin()->get_score(); ++opos)
			am_ac_winners[method][opos->get_candidate_num()][x][y]
				= true;
	}

	return(bottom_line);
}

bool yee::draw_pictures(string prefix, const vector<vector<vector<bool > > > &
		ac_winners, vector<vector<double> > & cand_colors, 
		vector<vector<double> > & cand_locations,
		bool draw_binaries_in, bool ignore_errors_in, 
		double inner_radius_in, double outer_radius_in, 
		double hue_factor) const {

	// First draw all the binary pictures (if so requested).
	// TODO: PNG yada yada. Don't have time for it now.

	size_t numcands = cand_colors.size(), counter, x, y,
	       xsize_in = ac_winners[0].size(), ysize_in = ac_winners[0][0].size();

	bool okay_so_far = true;
	string outfn;

	for (counter = 0; counter < numcands && draw_binaries_in; ++counter) {
		outfn = prefix + "_bin_c" + dtos(counter) + "won.pgm";
		ofstream outfile (outfn.c_str());

		if (!outfile) {
			okay_so_far = false;
			if (!ignore_errors_in && !okay_so_far) {
				cerr << "Couldn't open binary picture file "
					<< outfn << " for writing." << endl;
				return(false);
			}

			continue;
		}

		// Header!
		outfile << "P5 " << endl << xsize_in << " " << ysize_in << endl 
			<< 255 << endl;

		// Dump the boolean.
		for (y = 0; y < ysize_in; ++y)
			for (x = 0; x < xsize_in; ++x) {
				if (ac_winners[counter][x][y])
					outfile << (char) 255;
				else	outfile << (char) 0;
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
		return(false);
	}

	// Write the header.
	color_pic << "P6 " << endl << xsize_in << " " << ysize_in << endl << 255 
		<< endl;

	vector<double> adj_coords(2);

	// sqrt here etc.
	double adj_outer_radius = 2.5/(double)xsize_in + 2.5/(double)ysize_in;
	double adj_inner_radius = 1.5/(double)xsize_in + 1.5/(double)ysize_in;

	for (y = 0; y < ysize_in; ++y) {
		adj_coords[1] = y / (double)ysize_in;
		for (x = 0; x < xsize_in; ++x) {
			adj_coords[0] = x / (double)xsize_in;
			// If there's a tie, we output the mean color. If we
			// go to LAB at some later point, this may become more
			// complex.
			vector<double> prosp_RGB(3);
			int num_winners = 0;

			// We set these if any of the candidates have their
			// "home circle" around this pixel. It means we should
			// not draw anything but that candidate's home circle
			// at this point.
			int inner_border_of = -1;
			bool is_home = false;
			int copier;

			for (unsigned int cand = 0; cand < numcands && !is_home;
					++cand) {
				double dist = euc_distance(2.0, adj_coords, 
						cand_locations[cand]);

				if (dist < adj_outer_radius) {
					//outer_border_of = cand;
					is_home = true;
				}

				if (dist < adj_inner_radius)
					inner_border_of = cand;

				if (!ac_winners[cand][x][y] || is_home) 
					continue;
	
				for (copier = 0; copier < 3; ++copier)
					prosp_RGB[copier] += cand_colors[cand]
						[copier] * hue_factor;
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
					for (copier = 0; copier < 3; ++copier)
						prosp_RGB[copier] = 0;

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
	return(okay_so_far);

}

vector<vector<double> > yee::get_candidate_colors(int numcands, 
		bool debug) const {

	if (numcands <= 0)
		return(vector<vector<double> >());

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

	return(candidate_RGB);
}

std::string yee::get_codename(const election_method & in, int bytes) const {

	if (bytes > 16) {
		throw std::logic_error("yee::get_codename: "
			"Asking for more bytes than the hash provides");
	}

	std::string method_name = in.name();
	uint64_t hash_val[2] = {0, 0};

	SpookyHash::Hash128(method_name.c_str(), method_name.size(),
		&hash_val[0], &hash_val[1]);

	std::string outstr = ntos_hex(hash_val[0]) + ntos_hex(hash_val[1]);

	// Resize to the required byte count.
	outstr.resize(2 * bytes);

	return outstr;
}

// Public!

yee::yee() {

	// Set the parameters the user can't touch.

	inner_radius = 1.5; outer_radius = 2.5;
	color_attenuation_factor = 0.96;
	code_length = 5; // 40 bits should be enough for a long time.

	// These are definitely not true.
	specified_params = false;
	inited = false;
	manual_cand_positions = false;

	voter_pdf = NULL; candidate_pdf = NULL;
};

bool yee::set_params(int min_voters_in, int max_voters_in,
		int num_cands, bool do_use_autopilot, 
		double autopilot_factor_in, int autopilot_history_in,
		bool do_draw_binaries, string case_prefix, int xsize_in,
		int ysize_in, double sigma_in) {

	// Do some sanity checks. Bail if the user is giving us silly values.
	if (max_voters_in < min_voters_in || min_voters_in < 1)
		return(false);
	if (num_cands < 1)
		return(false);

	// A value less than 1 means it would try with fewer voters each time,
	// which makes no sense. Nor does 1 (stagnation) make sense.
	if (autopilot_factor_in <= 1) return(false);

	// A value of 1 here would mean that it accepts whatever the early
	// samples tell it.
	// Bluesky: Also, if greater than the maximum number of steps until
	// maxvoters, it makes no sense because that means it'll never fire.
	if (autopilot_history_in < 2) return(false);

	if (xsize_in < 1 || ysize_in < 1) return(false);
	if (sigma_in <= 0) return(false);

	// Okay, that seems reasonable enough - at least at first glance. Set.
	min_num_voters = min_voters_in;
	max_num_voters = max_voters_in;
	num_candidates = num_cands;
	use_autopilot = do_use_autopilot;
	autopilot_factor = autopilot_factor_in;
	autopilot_history_len = autopilot_history_in;
	draw_binaries = do_draw_binaries;

	// ?? Remove things like .. and / or nonprintables. Hm. Nah, the user 
	// should do that himself if he makes this part of a web service.
	run_prefix = case_prefix;

	x_size = xsize_in; y_size = ysize_in;
	sigma = sigma_in;

	specified_params = true;
	return(true);
}

bool yee::set_params(int num_voters, int num_cands, bool do_use_autopilot, 
		string case_prefix, int picture_size, double sigma_in) {

	// These parameters seem to work well: min_voters = min(24,
	// maxvoters), autopilot_factor 1.3, history length 4 (needs 4 in a
	// row with same winners before it accepts early), don't draw binaries,
	// xsize = ysize.

	return(set_params(min(24, num_voters), num_voters, num_cands,
				do_use_autopilot, 1.3, 4, false, case_prefix,
				picture_size, picture_size, sigma_in));
}

void yee::set_voter_pdf(spatial_generator * input_gen) {
	inited = false;
	voter_pdf = input_gen;
}

void yee::set_candidate_pdf(spatial_generator * input_gen) {
	inited = false;
	candidate_pdf = input_gen;
}

bool yee::set_candidate_positions(vector<vector<double> > & positions) {

	if (!specified_params)
		return(false);

	// See if the voter pdf accepts it.
	voter_pdf->set_params(2, true);
	bool accepted = voter_pdf->fix_candidate_positions(num_candidates, 
			positions);

	if (!accepted)
		return(false);

	inited = false;
	manual_cand_positions = true;
	return(true);
}

bool yee::randomize_candidate_positions(rng & randomizer) {

	// If the user hasn't specified any parameters, it's impossible to know
	// how many candidate positions to randomize!
	if (!specified_params)
		return(false);

	inited = false;
	voter_pdf->unfix_candidate_positions();
	if (!candidate_pdf->fix_candidate_positions(num_candidates, randomizer))
		return(false);
	// ???: Also do the move-to-voter-pdf thing here?

	manual_cand_positions = false;
	return(true);
}

void yee::add_method(const election_method * to_add) {
	inited = false;
	e_methods.push_back(to_add);
}

void yee::clear_methods() {
	inited = false;
	e_methods.clear();
}

bool yee::init(rng & randomizer) {

	//  If the user hasn't specified any parameters, no go.
	if (!specified_params) return(false);

	// If there are no voting methods or generators, we have nothing
	// to work with.
	if (voter_pdf == NULL || candidate_pdf == NULL || e_methods.empty())
		return(false);

	// Alright. Reset the winner arrays, get candidate colors, and
	// reset cur_round.
	winners_all_m_all_cand = vector<vector<vector<vector<bool> > > >(
			e_methods.size(), vector<vector<vector<bool> > >(
				num_candidates, vector<vector<bool> >(
					x_size, vector<bool>(y_size, 
						false))));

	candidate_colors = get_candidate_colors(num_candidates, false);
	cur_round = 0;

	// Set candidate positions if they haven't already been set manually.
	if (!manual_cand_positions) {
		candidate_pdf->set_params(2, true); // 2D

		if (!randomize_candidate_positions(randomizer)) {
			throw std::logic_error("Yee diagram: Could not randomize "
				"candidate positions!");
		}
	}

	// Port candidate positions over to the voter PDF.
	voter_pdf->set_params(2, true);
	if (!voter_pdf->fix_candidate_positions(num_candidates,
				candidate_pdf->get_fixed_candidate_pos())) {
		throw std::logic_error("Yee diagram: Could not fix "
			"candidate positions!");
	}
	// Set sigma.

	if (!voter_pdf->set_dispersion(sigma)) {
		throw std::logic_error("Yee diagram: Could not set standard "
			"deviation!");
	}

	inited = true;
	return(true);
}

// If we've inited, there's one round for each column as well as one round for
// each method at the end (to plot).
int yee::get_max_rounds() const {
	if (!inited) return(0);

	return(x_size + e_methods.size());
}

string yee::do_round(bool give_brief_status, bool reseed, rng & randomizer) {

	if (!inited) {
		throw std::runtime_error("Yee diagram: Needs to be "
			"initialized first");
	}

	string output;

	// Still determining points?
	if (cur_round < x_size) {
		output = "Yee: drawing x = " + itos(cur_round);

		long long grand_sum = 0;

		for (int y = 0; y < y_size; ++y) {
			long long contrib = check_pixel(cur_round, y, x_size, y_size,
					e_methods, *voter_pdf, 
					winners_all_m_all_cand,
					min_num_voters, max_num_voters,
					use_autopilot, autopilot_factor,
					autopilot_history_len, &cmap, 
					randomizer);

			if (contrib == -1) {
				cerr << "Yee: error at x " << cur_round << 
					", y " << y << endl;
				return("");
			}

			grand_sum += contrib;
		}

		output += ", " + lltos(grand_sum) + " voters in all.";
		++cur_round;
	} else {
		// No, draw the next picture.
		size_t method_no = cur_round - x_size;

		if (method_no >= e_methods.size())
			return(""); // All done!

		++cur_round;

		string code = get_codename(*e_methods[method_no], code_length);

		output = "Yee: " + e_methods[method_no]->name() + " has code "
			+ code + ". Drawing...";

		// Don't give up immediately if it's impossible to write a
		// certain file.
		bool ignore_file_errors = true;
		vector<vector<double> > candidate_posns = candidate_pdf->
			get_fixed_candidate_pos();

		if (draw_pictures(run_prefix + "_" + code, 
					winners_all_m_all_cand[method_no], 
					candidate_colors, candidate_posns, 
					draw_binaries, ignore_file_errors, 
					inner_radius, outer_radius, 
					color_attenuation_factor))
			output += "OK.";
		else	return(""); // Error. Should really be pair as these are really uninformative.
	}

	return(output);
}

vector<string> yee::provide_status() const {

	string out = "Yee: Done " + itos(cur_round) + " of " + itos(x_size + 
			e_methods.size()) + " rounds, or " + dtos(100.0 * 
			cur_round/(x_size + e_methods.size())) + "%";

	return(vector<string>(1, out));
}

