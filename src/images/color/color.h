// Auxiliary functions for converting between color representations. These are
// used for Yee diagrams.

// BLUESKY: Turn into a "colored pixel" class with gets and puts, and then just
// dump_to_rgb? No time for that now.

// WARNING: L*a*b* conversion gives weird results! Try with L = 1, for instance,
// and see max values > 1, and sometimes negative G,B values.
//	I now know why. L*ab is a device-independent color space. Therefore,
//	when converting between sRGB, some colors are "out of gamut", i.e.
//	the sRGB monitor can't draw them. Trying to convert out-of-gamut L*ab
//	values to sRGB then gives the bizarre results I saw. Unfortunately,
//	that pretty much destroys my idea of using L*ab for Yee -- that is,
//	unless I implement some form of gamut mapping to force the extreme
//	colors back into gamut.

#ifndef _VOTE_COLOR
#define _VOTE_COLOR

#include <vector>
#include <math.h>

enum color_space { CS_RGB, CS_HSV, CS_XYZ, CS_LAB}; // CS_LCh?

using namespace std;

class color_conv {

	private:
		// Should have *-to-RGB and RGB-to-*. Values are on [0...1].
		// Strategy patterns?
		// Other functions may be defined and then used by the RGB
		// ones (e.g LAB to LCh), but the conversion function goes from
		// source to RGB and then from RGB to dest.

		vector<double> HSV_to_RGB(const vector<double> HSV) const;
		vector<double> RGB_to_HSV(const vector<double> RGB) const;

		vector<double> XYZ_to_RGB(const vector<double> XYZ) const;
		vector<double> RGB_to_XYZ(vector<double> RGB) const;

		double labgamma(double x, double kappa, double eps) const;
		vector<double> XYZ_to_LAB(const vector<double> XYZ) const;
		vector<double> LAB_to_XYZ(const vector<double> LAB) const;

		vector<double> LAB_to_RGB(const vector<double> LAB) const {
			return (XYZ_to_RGB(LAB_to_XYZ(LAB)));
		}

		vector<double> RGB_to_LAB(const vector<double> RGB) const {
			return (XYZ_to_LAB(RGB_to_XYZ(RGB)));
		}

	public:

		vector<double> convert(const vector<double> & in,
			color_space from, color_space to) const;
};

#endif
