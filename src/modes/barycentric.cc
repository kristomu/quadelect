#include "barycentric.h"
#include <fstream>
#include <list>

#include "../ballots.h"
#include "../singlewinner/method.h"

#include "../spookyhash/SpookyV2.h"

ordering strict_ballot(std::string input) {

	ordering output;

	for (size_t counter = 0; counter < input.size(); ++counter)
		output.insert(candscore(input[counter] - 'A',
				input.size() - counter));

	return (output);
}

std::list<ballot_group>  barycentric::generate_ballot_set(double x,
	double y,
	double maxvoters, std::string first_group, std::string sec_group,
	std::string third_group, double x_1, double y_1, double x_2,
	double y_2, double x_3, double y_3) const {

	// Vertices of the triangle
	double l_1, l_2, l_3;

	double determinant = (y_2 - y_3) * (x_1 - x_3) + (x_3 - x_2) *
		(y_1 - y_3);

	l_1 = ((y_2 - y_3) * (x - x_3) + (x_3 - x_2) * (y - y_3)) / determinant;
	l_2 = ((y_3 - y_1) * (x - x_3) + (x_1 - x_3) * (y - y_3)) / determinant;
	l_3 = 1 - (l_1 + l_2);

	if (l_1 < 0 || l_2 < 0 || l_3 < 0) {
		return (std::list<ballot_group>());
	}

	std::list<ballot_group> toRet;
	toRet.push_back(ballot_group(l_1 * maxvoters, strict_ballot(
				first_group), true, false));
	toRet.push_back(ballot_group(l_2 * maxvoters, strict_ballot(
				sec_group), true, false));
	toRet.push_back(ballot_group(l_3 * maxvoters, strict_ballot(
				third_group), true, false));

	return (toRet);
}

std::list<ballot_group>  barycentric::generate_ballot_set(double x,
	double y,
	double maxvoters) const {

	// We have four groups:
	//	#1: (ABC, BCA, CAB): (0, 0) (0.5, 0), (0.25, 0.4)
	//	#2: (ABC, BAC, CAB): (0.5, 0.4), (1, 0.4), (0.75, 0)
	//	#3: (ABC, BAC, CBA): (0, 0.6) (0.5, 0.6) (0.25, 1)
	//	#4: (ABC, BCA, CBA): (0.5, 1), (1, 1), (0.75, 0.6)

	// These characterize the method's full response to fully specified
	// 3-candidate ballots, assuming neutrality and symmetry.

	// The three gropus are ABC, BCA, and CAB, so we can explore Condorcet
	// cycles in greater detail.

	std::list<ballot_group> ballots;

	if (ballots.empty())
		ballots = generate_ballot_set(x, y, maxvoters, "ABC", "BCA",
				"CAB", 0, 0, 0.5, 0, 0.25, 0.4);

	if (ballots.empty())
		ballots = generate_ballot_set(x, y, maxvoters, "ABC", "BAC",
				"CAB", 1, 0.4, 0.75, 0, 0.5, 0.4);
	//0.5, 0.4, 1, 0.4, 0.75, 0);

	if (ballots.empty())
		ballots = generate_ballot_set(x, y, maxvoters, "ABC", "BAC",
				"CAB", 0, 0.6, 0.5, 0.6, 0.25, 1);

	if (ballots.empty())
		ballots = generate_ballot_set(x, y, maxvoters, "ABC", "BCA",
				"CBA", 1, 1, 0.75, 0.6, 0.5, 1);
	//0.5, 1, 1, 1, 0.75, 0.6);

	return (ballots);
}

std::vector<std::vector<double> > barycentric::get_candidate_colors(
	int numcands,
	bool debug) const {

	if (numcands <= 0) {
		return (std::vector<std::vector<double> >());
	}

	std::vector<std::vector<double> > candidate_RGB(numcands);

	// The candidates are each given colors at hues spaced equally from
	// each other, and with full saturation and value. If you want
	// IEVS-style sphere packing, feel free to alter (call a class), but
	// it might be better in LAB color space -- if I could get LAB to work,
	// and if tone-mapping to limited gamut displays wasn't so hard.

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


std::string barycentric::get_codename(const election_method & in,
	size_t bytes) const {

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


// ---- //

void barycentric::add_method(const election_method * to_add) {
	inited = false;
	e_methods.push_back(to_add);
}

bool barycentric::init(rng & randomizer) {
	std::cout << "INIT 1" << std::endl;
	// If there are no methods, there's nothing we can do.
	if (e_methods.empty()) {
		return (false);
	}

	std::cout << "INIT 2" << std::endl;

	// Okay, set candidate colors. There are three candidates.
	cand_colors = get_candidate_colors(3, false);

	cur_round = 0;
	max_rounds = e_methods.size();
	return (true);
}

int barycentric::get_max_rounds() const {
	return (max_rounds);
}

int barycentric::get_current_round() const {
	return (cur_round);
}

std::string barycentric::do_round(bool give_brief_status, bool reseed,
	rng & randomizer) {

	std::cout << "DO_ROUND" << std::endl;

	// For the method in question:
	//	For every pixel,
	//		Generate a ballot set for the pixel (QND)
	//			If it's empty, paint white
	//			Otherwise go through the method to find the
	//			winner/s, and plot them.

	if (cur_round >= max_rounds) {
		return ("");
	}

	const election_method * our_method = e_methods[cur_round];

	int num_cands = 3;	// Because of barycentric rendering.

	// TODO: Make parameters.
	int xsize = 400;
	int ysize = 400;
	int code_length = 5;

	double numvoters = 100;

	std::string code = get_codename(*our_method, code_length);
	std::string status = "Barycentric: " + our_method->name() + " has code "
		+ code + ". Drawing...";

	std::string outfn = code + "_bary.ppm";
	std::ofstream color_pic(outfn.c_str());

	if (!color_pic) {
		// Couldn't open that file.
		std::cerr << "Couldn't open color picture file " << outfn <<
			" for writing." << std::endl;

		// Since we've got nothing more to do, we can just return false
		// no matter what here.
		return (status + "Error!");
	}

	// Write the header
	color_pic << "P6 " << std::endl << xsize << " " << ysize << std::endl <<
		255 <<
		std::endl;

	for (int y = 0; y < ysize; ++y) {
		for (int x = 0; x < xsize; ++x) {
			std::list<ballot_group> barycentric_ballot =
				generate_ballot_set(x / (double)xsize,
					y / (double)ysize,
					numvoters);

			if (barycentric_ballot.empty()) {
				// Draw white
				color_pic << (char)255 << (char)255
					<< (char)255;
			} else {
				// Get the winner/s and determine its color.

				ordering soc_ranking = our_method->elect(
						barycentric_ballot, num_cands,
						NULL, true);

				// HACK. TODO: FIX.
				int winner = soc_ranking.begin()->
					get_candidate_num();

				for (int i = 0; i < 3; ++i)
					color_pic << (char)(round(255.4999 *
								cand_colors[
									winner][i]));
			}
		}
	}

	++cur_round;

	return (status + "OK");
}

std::vector<std::string> barycentric::provide_status() const {
	std::string out = "Barycentric: Done " + itos(cur_round) + " of " +
		itos(max_rounds) + " rounds, or " +
		dtos(100.0 * cur_round/(double)max_rounds) + "%";

	return (std::vector<std::string>(1, out));
}
