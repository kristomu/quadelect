// Mode for drawing Yee diagrams. Yee diagrams consist of plotting, for each
// pixel in a picture (translated to a point in [0...1]^2), who wins if the
// voters are modeled as a normal distribution centered on that point, and each
// voter prefers a candidate nearby to a candidate further away.

// Currently only supports writing to PGM and PPM formats. PNG may come later.

#ifndef _VOTE_MODE_YEE
#define _VOTE_MODE_YEE

#include "mode.h"
#include "../ballots.h"
#include "../singlewinner/method.h"
#include "../generator/spatial/gaussian.h"

#include "../spookyhash/SpookyV2.h"

class yee : public mode {

	private:
		bool specified_params, inited, manual_cand_positions;
		int cur_round;

		// Parameters.
		int min_num_voters, max_num_voters, num_candidates;
		bool use_autopilot; double autopilot_factor;
		int autopilot_history_len;
		bool draw_binaries;
		string run_prefix;// Prefix to give pictures. Keep it short!
		int x_size, y_size;
		double sigma;     // Desired standard deviation of the Gaussian.

		// Parameters that aren't available to the public.
		double inner_radius, outer_radius; // for candidate home bases
		double color_attenuation_factor;   // to make bases stand out.

		// SHA code length for method names, in bytes. If there are
		// 2^n methods in all (even those you aren't using), set it
		// to (2n)/8 at least (by birthday paradox).
		int code_length;

		// Election methods and generators.
		spatial_generator * voter_pdf, * candidate_pdf;
		vector<const election_method *> e_methods;

		// Arrays that hold information about who won and the color
		// corresponding to each candidate.
		vector<vector<vector<vector<bool > > > > winners_all_m_all_cand;
		vector<vector<double> > candidate_colors;

		// For caching.
		cache_map cmap;

		// This function generates hex-style "codenames" for each method
		// so the mode doesn't overwrite pictures when dealing with more
		// than one method at a time. It's generated from a hash function.
		string get_codename(const election_method & in,
			int bytes) const;

		// Produce candidate colors, given the number of candidates.
		vector<vector<double> > get_candidate_colors(int numcands,
			bool debug) const;

		// Test a given pixel and update winners arrays. See the .cc
		// for more information.
		long long check_pixel(int x, int y, int xsize, int ysize,
			const vector<const election_method *> & methods,
			spatial_generator & ballotgen,
			vector<vector<vector<vector<bool > > > > &
			am_ac_winners, int min_num_voters_in,
			int max_num_voters_in, bool do_use_autopilot,
			double autopilot_factor_in,
			int autopilot_history_in,
			cache_map * cache, rng & randomizer) const;

		// Given complete winners arrays, draw the different pictures
		// that visualize those arrays.
		bool draw_pictures(string prefix,
			const vector<vector<vector<bool > > > &
			ac_winners,
			vector<vector<double> > & cand_colors,
			vector<vector<double> > & cand_locations,
			bool draw_binaries, bool ignore_errors,
			double inner_radius_in, double outer_radius_in,
			double hue_factor) const;

	public:

		yee();

		// For advanced use.
		bool set_params(int min_voters_in, int max_voters_in,
			int num_cands, bool do_use_autopilot, double
			autopilot_factor_in, int autopilot_history_len,
			bool do_draw_binaries, string case_prefix,
			int xsize_in, int ysize_in, double sigma_in);

		// Reasonable defaults for the technical params.
		bool set_params(int num_voters, int num_cands,
			bool do_use_autopilot, string case_prefix,
			int picture_size, double sigma_in);

		// These void init.
		void set_voter_pdf(spatial_generator * candidate);
		void set_candidate_pdf(spatial_generator * candidate);
		bool set_candidate_positions(vector<vector<double> > &
			positions);
		bool randomize_candidate_positions(rng & randomizer);

		void add_method(const election_method * to_add);
		template<typename T> void add_methods(T start_iter, T end_iter);
		void clear_methods();

		bool init(rng & randomizer); // Will reinit if already inited.

		// Each round is one column. Having each round be a point and
		// then having to return a string would get real expensive
		// real fast.

		// If not inited, who knows? If inited, xsize + num_methods.
		int get_max_rounds() const;
		int get_current_round() const {
			return (cur_round);
		}

		// Reseed does nothing here.
		string do_round(bool give_brief_status, bool reseed,
			rng & randomizer);

		vector<string> provide_status() const;
};

template<typename T> void yee::add_methods(T start_iter, T end_iter) {

	for (T iterator = start_iter; iterator != end_iter; ++iterator) {
		add_method(*iterator);
	}
}

#endif
