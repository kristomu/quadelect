// Mode for drawing Yee diagrams. Yee diagrams consist of plotting, for each
// pixel in a picture (translated to a point in [0...1]^2), who wins if the
// voters are modeled as a normal distribution centered on that point, and each
// voter prefers a candidate nearby to a candidate further away.

#include "yee.h"
#include "../images/color/color.h"
#include "../output/png_writer.h"
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
	const std::vector<std::shared_ptr<const election_method> > & methods,
	spatial_generator & ballotgen,
	std::vector<std::vector<std::vector<std::vector<bool > > > > &
	am_ac_winners,
	int min_num_voters_in, int max_num_voters_in,
	bool use_autopilot_in, double autopilot_factor_in,
	int autopilot_history_in, cache_map * cache,
	coordinate_gen & ballot_coord_source) const {

	size_t num_cands = am_ac_winners[0].size(),
		   num_methods = am_ac_winners.size();

	std::vector<std::list<ballot_group> > autopilot_am(num_methods);
	std::vector<int> num_identical(num_methods, 0);
	std::list<ballot_group> ballots;

	// Arrange these so (0,0) is at the bottom left?
	std::vector<double> relative(2);
	relative[0] = x / (double) xsize_in;
	relative[1] = y / (double) ysize_in;

	if (!ballotgen.set_center(relative)) {
		return (-1);
	}

	double cur_num_voters = min_num_voters_in;

	if (!use_autopilot_in) {
		cur_num_voters = max_num_voters_in;
	}

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
				num_cands, ballot_coord_source);

		cache->clear();

		for (method = 0; method < num_methods && cleared < num_methods;
			++method) {
			// If we've already got enough data to say who this
			// method elects, ignore it.
			if (num_identical[method] >= autopilot_history_in) {
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
				if (num_identical[method] >= autopilot_history_in) {
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

		if (cur_num_voters != max_num_voters)
			cur_num_voters = std::min((double)max_num_voters,
					cur_num_voters * std::max(def_autopilot_factor,
						autopilot_factor_in));
		else
			cur_num_voters *= std::max(def_autopilot_factor,
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

	return (bottom_line);
}

void yee::draw_pictures(std::string prefix,
	std::string method_name, uint64_t seed,
	const std::vector<std::vector<std::vector<bool > > > &
	ac_winners, std::vector<std::vector<double> > & cand_colors,
	std::vector<std::vector<double> > & cand_locations,
	double inner_radius_in, double outer_radius_in,
	double hue_factor) const {

	// Build the color picture.

	png_writer picture_out(prefix + "_yee.png", x_size, y_size);

	std::vector<double> adj_coords(2);

	// sqrt here etc.
	double adj_outer_radius = 2.5/(double)x_size + 2.5/(double)y_size;
	double adj_inner_radius = 1.5/(double)x_size + 1.5/(double)y_size;

	for (int y = 0; y < y_size; ++y) {
		adj_coords[1] = y / (double)y_size;
		for (int x = 0; x < x_size; ++x) {
			adj_coords[0] = x / (double)x_size;
			// If there's a tie, we output the mean color. If we
			// go to LAB at some later point, this may become more
			// complex.
			std::vector<double> prospective_pixel(3);
			int num_winners = 0;

			// We set these if any of the candidates have their
			// "home circle" around this pixel. It means we should
			// not draw anything but that candidate's home circle
			// at this point.
			int inner_border_of = -1;
			bool is_home = false;
			int color_idx;

			for (int cand = 0; cand < num_candidates
				&& !is_home; ++cand) {
				double dist = euc_distance(2.0, adj_coords,
						cand_locations[cand]);

				if (dist < adj_outer_radius) {
					//outer_border_of = cand;
					is_home = true;
				}

				if (dist < adj_inner_radius) {
					inner_border_of = cand;
				}

				if (!ac_winners[cand][x][y] || is_home) {
					continue;
				}

				for (color_idx = 0; color_idx < 3; ++color_idx)
					prospective_pixel[color_idx] += cand_colors[cand]
						[color_idx] * hue_factor;
				++num_winners;
			}

			// Check if it's the home of someone. If so, color
			// appropriately (black border if outer, color if
			// inner). If not, go ahead and plunk down our pixel.

			if (is_home) {
				if (inner_border_of != -1) {
					prospective_pixel = cand_colors[inner_border_of];
				} else
					for (color_idx = 0; color_idx < 3; ++color_idx) {
						prospective_pixel[color_idx] = 0;
					}

				num_winners = 1;
			}

			for (color_idx = 0; color_idx < 3; ++color_idx) {
				prospective_pixel[color_idx] /= (double)num_winners;
			}

			picture_out.put_pixel(x, y, prospective_pixel);
		}
	}

	picture_out.add_text("Voting method", method_name);
	picture_out.add_text("Picture type", "Yee diagram");
	picture_out.add_text("RNG seed", gen_itos(seed));
	picture_out.finalize();
}

std::vector<std::vector<double> > yee::get_candidate_colors(int numcands,
	bool debug) const {

	if (numcands <= 0) {
		return (std::vector<std::vector<double> >());
	}

	std::vector<std::vector<double> > candidate_RGB(numcands);

	// The candidates are each given colors at hues spaced equally from
	// each other, and with full saturation and value. If you want
	// IEVS-style sphere packing, feel free to alter (call a class), but
	// it might be better in LAB color space -- if I could get LAB to work.

	std::vector<double> HSV(3, 1);
	HSV[0] = 0;

	color_conv converter;

	for (int cand = 0; cand < numcands; ++cand) {
		candidate_RGB[cand] = converter.convert(HSV, CS_HSV, CS_RGB);

		if (debug) {
			std::cout << "RGB values for " << cand << ": ";
			copy(candidate_RGB[cand].begin(),
				candidate_RGB[cand].end(),
				std::ostream_iterator<double>(std::cout, "\t"));

			std::cout << std::endl;
		}

		// It must be +1 because the Hue aspect is circular, leaving
		// just as much space between the last candidate's hue and the
		// first, as between the first and second's.

		HSV[0] += 1 / (double)(numcands+1);
	}

	return (candidate_RGB);
}

std::string yee::get_codename(const election_method & in,
	int bytes) const {

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
	std::string case_prefix, int xsize_in,
	int ysize_in, double sigma_in) {

	// Do some sanity checks. Bail if the user is giving us silly values.
	if (max_voters_in < min_voters_in || min_voters_in < 1) {
		return (false);
	}
	if (num_cands < 1) {
		return (false);
	}

	// A value less than 1 means it would try with fewer voters each time,
	// which makes no sense. Nor does 1 (stagnation) make sense.
	if (autopilot_factor_in <= 1) {
		return (false);
	}

	// A value of 1 here would mean that it accepts whatever the early
	// samples tell it.
	// Bluesky: Also, if greater than the maximum number of steps until
	// maxvoters, it makes no sense because that means it'll never fire.
	if (autopilot_history_in < 2) {
		return (false);
	}

	if (xsize_in < 1 || ysize_in < 1) {
		return (false);
	}
	if (sigma_in <= 0) {
		return (false);
	}

	// Okay, that seems reasonable enough - at least at first glance. Set.
	min_num_voters = min_voters_in;
	max_num_voters = max_voters_in;
	num_candidates = num_cands;
	use_autopilot = do_use_autopilot;
	autopilot_factor = autopilot_factor_in;
	autopilot_history_len = autopilot_history_in;

	// ?? Remove things like .. and / or nonprintables. Hm. Nah, the user
	// should do that himself if he makes this part of a web service.
	run_prefix = case_prefix;

	x_size = xsize_in; y_size = ysize_in;
	sigma = sigma_in;

	specified_params = true;
	return (true);
}

bool yee::set_params(int num_voters, int num_cands, bool do_use_autopilot,
	std::string case_prefix, int picture_size, double sigma_in) {

	// These parameters seem to work well: min_voters = std::min(24,
	// maxvoters), autopilot_factor 1.3, history length 4 (needs 4 in a
	// row with same winners before it accepts early), don't draw binaries,
	// xsize = ysize.

	return (set_params(std::min(24, num_voters), num_voters, num_cands,
				do_use_autopilot, 1.3, 4, case_prefix,
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

bool yee::set_candidate_positions(std::vector<std::vector<double> > &
	positions) {

	if (!specified_params) {
		return (false);
	}

	// See if the voter pdf accepts it.
	voter_pdf->set_params(2, true);
	bool accepted = voter_pdf->fix_candidate_positions(num_candidates,
			positions);

	if (!accepted) {
		return (false);
	}

	inited = false;
	manual_cand_positions = true;
	return (true);
}

bool yee::randomize_candidate_positions(
	coordinate_gen & candidate_coord_source) {

	// If the user hasn't specified any parameters, it's impossible to know
	// how many candidate positions to randomize!
	if (!specified_params) {
		return false;
	}

	inited = false;
	voter_pdf->unfix_candidate_positions();
	if (!candidate_pdf->fix_candidate_positions(num_candidates,
			candidate_coord_source)) {
		return false;
	}

	set_initial_seed(candidate_coord_source); // XXX: HACK!
	manual_cand_positions = false;
	return true;
}

void yee::add_method(std::shared_ptr<const election_method> to_add) {
	inited = false;
	e_methods.push_back(to_add);
}

void yee::clear_methods() {
	inited = false;
	e_methods.clear();
}

bool yee::init(coordinate_gen & candidate_coord_source) {

	//  If the user hasn't specified any parameters, no go.
	if (!specified_params) {
		return (false);
	}

	// If there are no voting methods or generators, we have nothing
	// to work with.
	if (voter_pdf == NULL || candidate_pdf == NULL || e_methods.empty()) {
		return (false);
	}

	// Alright. Reset the winner arrays, get candidate colors, and
	// reset cur_round.
	winners_all_m_all_cand =
		std::vector<std::vector<std::vector<std::vector<bool> > > >(
			e_methods.size(), std::vector<std::vector<std::vector<bool> > >(
				num_candidates, std::vector<std::vector<bool> >(
					x_size, std::vector<bool>(y_size,
						false))));

	candidate_colors = get_candidate_colors(num_candidates, false);
	cur_round = 0;

	// Set candidate positions if they haven't already been set manually.
	if (!manual_cand_positions) {
		candidate_pdf->set_params(2, true); // 2D

		if (!randomize_candidate_positions(candidate_coord_source)) {
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

	// Each round, we draw a new row. However, if we want to have
	// accurate predictions of the time needed to finish, there should
	// be no consistent bias (e.g. top-heavy rounds). So create a random
	// mapping of rows to round numbers so that the picture will be drawn in
	// a random order.
	round_row_mapping.resize(x_size);
	std::iota(round_row_mapping.begin(), round_row_mapping.end(), 0);
	std::random_shuffle(round_row_mapping.begin(), round_row_mapping.end());

	return (true);
}

// If we've inited, there's one round for each column as well as one round for
// each method at the end (to plot).
int yee::get_max_rounds() const {
	if (!inited) {
		return (0);
	}

	return (x_size + e_methods.size());
}

std::string yee::do_round(bool give_brief_status, bool reseed,
	coordinate_gen & ballot_coord_source) {

	if (!inited) {
		throw std::runtime_error("Yee diagram: Needs to be "
			"initialized first");
	}

	std::string output;

	// Still determining points?
	if (cur_round < x_size) {
		int row_number = round_row_mapping[cur_round];

		output = "Yee: round " + itos(cur_round) + "/" + itos(get_max_rounds())
			+ ": drawing x = " + itos(row_number);

		long long grand_sum = 0;

		for (int y = 0; y < y_size; ++y) {
			long long contrib = check_pixel(row_number, y, x_size, y_size,
					e_methods, *voter_pdf,
					winners_all_m_all_cand,
					min_num_voters, max_num_voters,
					use_autopilot, autopilot_factor,
					autopilot_history_len, &cmap,
					ballot_coord_source);

			if (contrib == -1) {
				std::cerr << "Yee: error at round " << cur_round
					<< ", x = " << row_number << ", y = " << y << std::endl;
				return ("");
			}

			grand_sum += contrib;
		}

		output += ", " + lltos(grand_sum) + " voters in all.";
		++cur_round;
	} else {
		// No, output the picture to disk.
		size_t method_no = cur_round - x_size;

		if (method_no >= e_methods.size()) {
			return ("");    // All done!
		}

		++cur_round;

		std::string code = get_codename(*e_methods[method_no], code_length);

		std::string method_name = e_methods[method_no]->name();

		output = "Yee: " + method_name + " has code " + code + ". Drawing...";

		std::vector<std::vector<double> > candidate_posns = candidate_pdf->
			get_fixed_candidate_pos();

		// Don't give up immediately if it's impossible to write a
		// certain file.

		try {
			draw_pictures(run_prefix + "_" + code,
				method_name, initial_seed,
				winners_all_m_all_cand[method_no],
				candidate_colors, candidate_posns,
				inner_radius, outer_radius,
				color_attenuation_factor);

			output += "OK.";
		} catch (std::runtime_error & re) {
			// Error. We should really be returning error status *and*
			// explanatory text as just "error if it's empty, otherwise
			// OK" is really uninformative.
			return "";
		}
	}

	return output;
}

std::vector<std::string> yee::provide_status() const {

	std::string out = "Yee: Done " + itos(cur_round) + " of " + itos(x_size +
			e_methods.size()) + " rounds, or " + dtos(100.0 *
			cur_round/(x_size + e_methods.size())) + "%";

	return (std::vector<std::string>(1, out));
}

