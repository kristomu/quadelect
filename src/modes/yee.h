// Mode for drawing Yee diagrams. Yee diagrams consist of plotting, for each
// pixel in a picture (translated to a point in [0...1]^2), who wins if the
// voters are modeled as a normal distribution centered on that point, and each
// voter prefers a candidate nearby to a candidate further away.

#pragma once

#include "mode.h"
#include "../ballots.h"
#include "../singlewinner/method.h"
#include "../generator/spatial/gaussian.h"

#include "../spookyhash/SpookyV2.h"

#include <memory>

class yee : public mode {

	private:
		bool specified_params, inited, manual_cand_positions;
		int cur_round;

		// Parameters.
		int min_num_voters, max_num_voters, num_candidates;
		bool use_autopilot; double autopilot_factor;
		int autopilot_history_len;
		std::string run_prefix;// Prefix to give pictures. Keep it short!
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
		std::vector<std::shared_ptr<const election_method> > e_methods;

		// Arrays that hold information about who won and the color
		// corresponding to each candidate.
		std::vector<std::vector<std::vector<std::vector<bool > > > >
		winners_all_m_all_cand;
		std::vector<std::vector<double> > candidate_colors;

		// For caching.
		cache_map cmap;

		// Round-row mapping to remove bias when calculating ETA.
		std::vector<double> round_row_mapping;

		// This function generates hex-style "codenames" for each method
		// so the mode doesn't overwrite pictures when dealing with more
		// than one method at a time. It's generated from a hash function.
		std::string get_codename(const election_method & in,
			int bytes) const;

		// Produce candidate colors, given the number of candidates.
		std::vector<std::vector<double> > get_candidate_colors(int numcands,
			bool debug) const;

		// Test a given pixel and update winners arrays. See the .cc
		// for more information.
		long long check_pixel(int x, int y, int xsize, int ysize,
			const std::vector<std::shared_ptr<const election_method> > & methods,
			spatial_generator & ballotgen,
			std::vector<std::vector<std::vector<std::vector<bool > > > > &
			am_ac_winners, int min_num_voters_in,
			int max_num_voters_in, bool do_use_autopilot,
			double autopilot_factor_in,
			int autopilot_history_in, cache_map * cache,
			coordinate_gen & ballot_coord_source) const;

		// Given complete winners arrays, draw the different pictures
		// that visualize those arrays. The method name and RNG seed
		// are added to the picture as text metadata for archiving etc.
		void draw_pictures(std::string prefix,
			std::string method_name, uint64_t seed,
			const std::vector<std::vector<std::vector<bool > > > &
			ac_winners,
			std::vector<std::vector<double> > & cand_colors,
			std::vector<std::vector<double> > & cand_locations,
			double inner_radius_in, double outer_radius_in,
			double hue_factor) const;

		bool is_valid_purpose(uint32_t purpose) const {
			return purpose == PURPOSE_BALLOT_GENERATOR
				|| purpose == PURPOSE_CANDIDATE_DATA;
		}

		using mode::init;
		bool init(coordinate_gen & candidate_coord_source);

	public:

		yee();

		// For advanced use.
		bool set_params(int min_voters_in, int max_voters_in,
			int num_cands, bool do_use_autopilot, double
			autopilot_factor_in, int autopilot_history_len,
			std::string case_prefix, int xsize_in, int ysize_in,
			double sigma_in);

		// Reasonable defaults for the technical params.
		bool set_params(int num_voters, int num_cands,
			bool do_use_autopilot, std::string case_prefix,
			int picture_size, double sigma_in);

		// These void init.
		void set_voter_pdf(spatial_generator * candidate);
		void set_candidate_pdf(spatial_generator * candidate);
		bool set_candidate_positions(std::vector<std::vector<double> > &
			positions);

		bool set_candidate_positions(coordinate_gen &
			candidate_coord_source);
		bool set_candidate_positions();

		void add_method(std::shared_ptr<const election_method> to_add);
		template<typename T> void add_methods(T start_iter, T end_iter);
		void clear_methods();

		// Will reinit if already inited.
		bool init();

		// Each round is one column. Having each round be a point and
		// then having to return a string would get real expensive
		// real fast.

		// If not inited, who knows? If inited, xsize + num_methods.
		int get_max_rounds() const;
		int get_current_round() const {
			return cur_round;
		}

		std::string do_round(bool give_brief_status);

		std::vector<std::string> provide_status() const;

		std::string name() const {
			return "Yee renderer";
		}
};

template<typename T> void yee::add_methods(T start_iter, T end_iter) {

	for (T iterator = start_iter; iterator != end_iter; ++iterator) {
		add_method(*iterator);
	}
}
