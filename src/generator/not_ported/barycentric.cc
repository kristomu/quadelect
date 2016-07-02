// This generator sets a random point within a triangle, then generates a
// ballot list of the form
// x: A > B > C
// y: B > C > A
// z: C > A > B
// based on treating that point as a barycentric coordinate.
// As we intend to use this for a ballot generator to draw Spruced-Up type
// diagrams, it also permits the user to provide such a point, explicitly. It
// returns an empty list if the point is outside the triangle.

// Conversion equations:
// l_1 = (y2-y3)(x-x3) - (x2-x3)(y-y3)
// l_2 = -(y1-y3)(x-x3) + (x1-x3)(y-y3)
// l_3 = 1 - l_1 - l_2
// and the other way around:
// x = l_1 * x1 + l_2 * x2 + l_3 * x3
// y = l_1 * y1 + l_2 * y2 + l_3 * y3

// Make ballot generation virtual, too?

#include <iostream>
#include <vector>
#include <assert.h>

#include "ballotgen.cc"
//#include "../method.cc"

// Not ported yet. Idea: generate_ordering takes a v<d>::const_iterator & params
// and there's also a generate_random function that fills this v<d> with as
// many randoms as is required. BLUESKY: Implement.

class barycentric : public pure_ballot_generator {
	protected:
		virtual ordering generate_ordering(int numcands,
				bool do_truncate) const = 0;
};

using namespace std;

// These are the reference points - the vertices of the triangle.
vector<pair<double, double> > reference_points() {
	vector<pair<double, double> > out;

	// first point is at (0.5, 0) (top middle)
	// second point is at (0, 1) (lower left)
	// third point is at (1, 1) (lower right)
	
	out.push_back(pair<double, double>(0.5, 0));
	out.push_back(pair<double, double>(0, 1));
	out.push_back(pair<double, double>(1, 1));

	return(out);
}

// Transform a Cartesian coordinate to a barycentric one.
vector<double> from_cartesian(const pair<double, double> & coords) {
	assert(coords.first >= 0 && coords.first <= 1);
	assert(coords.second >= 0 && coords.second <= 1);

	vector<double> out(3);
	vector<pair<double, double> > base = reference_points();

	// l_1 = (y2-y3)(x-x3) - (x2-x3)(y-y3)
	// l_2 = -(y1-y3)(x-x3) + (x1-x3)(y-y3)
	// l_3 = 1 - l_1 - l_2

	double y1 = base[0].second, y2 = base[1].second,
	       y3 = base[2].second;
	double x1 = base[0].first, x2 = base[1].first,
	       x3 = base[2].first;

	out[0] = (y2 - y3) * (coords.first - x3) - 
		(x2 - x3) * (coords.second - y3); 

	out[1] = -(y1 - y3) * (coords.first - x3) +
		(x1 - x3) * (coords.second - y3);

	// DET?
	out[0] /= -1.0;
	out[1] /= -1.0;

	out[2] = 1 - out[0] - out[1];

	return(out);
}

bool is_inside(const vector<double> & bary_coords) {

	for (int counter = 0; counter < bary_coords.size(); ++counter)
		if (bary_coords[counter] < 0 || bary_coords[counter] > 1)
			return(false);

	return(true);
}

pair<double, double> to_cartesian(const vector<double> & input) {

	vector<pair<double, double> > base = reference_points();

	pair<double, double> cartesian;

	// x = l_1 * x1 + l_2 * x2 + l_3 * x3
	// y = l_1 * y1 + l_2 * y2 + l_3 * y3
	
	cartesian.first = input[0] * base[0].first + input[1] * base[1].first +
		input[2] * base[2].first;
	cartesian.second = input[0] * base[0].second + 
		input[1] * base[1].second + input[2] * base[2].second;

	return(cartesian);
}

main() {

	pair<double, double> init;
	init.first = 0.5;
	init.second = 0.3;

	vector<double> converted = from_cartesian(init);

	cout << converted[0] << "\t" << converted[1] << "\t" << converted[2] 
		<< endl;

	init = to_cartesian(converted);

	cout << init.first << "\t" << init.second << endl;
}
