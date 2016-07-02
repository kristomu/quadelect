// Auxiliary functions for converting between color representations. These are
// used for Yee diagrams.

// WARNING: L*a*b* conversion gives weird results! Try with L = 1, for instance,
// and see max values > 1, and sometimes negative G,B values. I don't know why,
// so use at your own risk.

#include "color.h"
#include <iostream>

using namespace std;

vector<double> color_conv::HSV_to_RGB(const vector<double> HSV) const {

	double m, n, f;
	int i;

	// Old code.
	double h = HSV[0], s = HSV[1], v = HSV[2];

	h *= 6;

	if (s == 0)
		return(vector<double>(3, v)); // No color, so it's all value.

	i = floor(h);
	f = h - i;
        if ( !(i&1) ) f = 1 - f; // if i is even 
        m = v * (1 - s);
        n = v * (1 - s * f);

	vector<double> RGB(3);

        switch (i) {
                case 6:
                case 0: RGB[0] = v; RGB[1] = n; RGB[2] = m; break;
                case 1: RGB[0] = n; RGB[1] = v; RGB[2] = m; break;
                case 2: RGB[0] = m; RGB[1] = v; RGB[2] = n; break;
                case 3: RGB[0] = m; RGB[1] = n; RGB[2] = v; break;
                case 4: RGB[0] = n; RGB[1] = m; RGB[2] = v; break;
                case 5: RGB[0] = v; RGB[1] = m; RGB[2] = n; break;
        }

	return(RGB);
}

vector<double> color_conv::RGB_to_HSV(const vector<double> RGB) const {
	// By convention, we set h to 0 if s is 0.

	double minval, maxval, delta;

	vector<double> HSV(3);

	minval = min(RGB[0], min(RGB[1], RGB[2]));
	maxval = max(RGB[0], min(RGB[1], RGB[2]));
	HSV[2] = maxval; // v

	delta = maxval - minval;

	if (maxval != 0) {
		HSV[1] = delta / maxval;		// saturation
	} else {
		// R = G = B, and they're all zero.
		HSV[1] = 0;
		HSV[0] = 0;
		return(HSV);
	}

	if (RGB[0] == maxval) {
		HSV[0] = (RGB[1] - RGB[2]) / delta; // between yellow & magenta
	} else if (RGB[1] == maxval) {
		HSV[0] = 2 + (RGB[2] - RGB[0]) / delta; // between cyan & yellow
	} else
		HSV[0] = 4 + (RGB[0] - RGB[1]) / delta; // between mag. and cyan

	HSV[0] /= 6.0; // 0...1

	if (HSV[0] < 0)
		++HSV[0];

	return(HSV);
}

double color_conv::labgamma(double x, double kappa, double eps) const {
	if (x > eps) return(pow(x, 1/3.0));

	return((kappa*x + 16)/116.0);
}

vector<double> color_conv::XYZ_to_RGB(const vector<double> XYZ) const {

	assert(XYZ.size() >= 3);

	vector<double> RGB(3);

	RGB[0] =  3.240479 * XYZ[0] + -1.537150 * XYZ[1] + -0.498535 * XYZ[2];
	RGB[1] = -0.969256 * XYZ[0] +  1.875992 * XYZ[1] +  0.041556 * XYZ[2];
	RGB[2] =  0.055648 * XYZ[0] + -0.204043 * XYZ[1] +  1.057311 * XYZ[2];

	return(RGB);
}

vector<double> color_conv::RGB_to_XYZ(const vector<double> RGB) const {

	vector<double> XYZ(3);

	XYZ[0] =  0.412453 * RGB[0] + 0.357580 * RGB[1] + 0.180423 * RGB[2];
	XYZ[1] =  0.212671 * RGB[0] + 0.715160 * RGB[1] + 0.072169 * RGB[2];
	XYZ[2] =  0.019334 * RGB[0] + 0.119193 * RGB[1] + 0.950227 * RGB[2];

	return(XYZ);
}

vector<double> color_conv::XYZ_to_LAB(const vector<double> XYZ) const {
	// D65 reference white. Perhaps I should use D50 instead.

	double xadj = XYZ[0] / 0.95047, 
	       yadj = XYZ[1], 
	       zadj = XYZ[2] / 1.08883;

	double kappa = 24389/27.0, eps = 216/24389.0;
	
	double fx = labgamma(xadj, kappa, eps);
        double fy = labgamma(yadj, kappa, eps);
	double fz = labgamma(zadj, kappa, eps);

	vector<double> LAB(3);

	LAB[0] = 116 * fy - 16;
	LAB[1] = 500 * (fx - fy);
	LAB[2] = 200 * (fy - fz);

	// Custom conversion. LAB is usually on [0...100, -110, +110, 
	// -110, +110]. After conversion, they are on [0...1, -1...1, -1...1].

	LAB[0] /= 100.0;
	LAB[1] /= 110.0;
	LAB[2] /= 110.0;

	return(LAB);
}

vector<double> color_conv::LAB_to_XYZ(const vector<double> LAB) const {

	assert(LAB.size() >= 3);

	// Custom conversion. See XYZ_to_LAB.

	vector<double> cLAB = LAB;
	cLAB[0] *= 100.0;
	cLAB[1] *= 110.0;
	cLAB[2] *= 110.0;

        double var_Y = ( cLAB[0] + 16 ) / 116;
        double var_X = cLAB[1] / 500 + var_Y;
        double var_Z = var_Y - cLAB[2] / 200;

        double X, Y, Z;

        if ( pow(var_Y,3) > 0.008856 ) Y = pow(var_Y,3);
        else                      Y = ( var_Y - 16 / 116 ) / 7.787;
        if ( pow(var_X,3) > 0.008856 ) X = pow(var_X,3);
        else                      X = ( var_X - 16 / 116 ) / 7.787;
        if ( pow(var_Z,3) > 0.008856 ) Z = pow(var_Z,3);
        else                      Z = ( var_Z - 16 / 116 ) / 7.787;

	vector<double> XYZ(3);

        // D65 reference point
	XYZ[0] = X * 0.95047;
	XYZ[1] = Y * 1;
	XYZ[2] = Z * 1.08883;
	
	return(XYZ);
}

vector<double> color_conv::convert(const vector<double> & in,
                color_space from, color_space to) const {

        if (from == to) return(in);

        vector<double> translator;

        switch (from) {
                case CS_RGB:
                        translator = in;
                        break;
                case CS_HSV:
                        translator = HSV_to_RGB(in);
                        break;
                case CS_XYZ:
                        translator = XYZ_to_RGB(in);
			break;
		case CS_LAB:
			translator = LAB_to_RGB(in);
			break;
                default:
                        assert (1 != 1);
        }

        switch(to) {
                case CS_RGB:
                        return(translator);
                case CS_HSV:
                        return(RGB_to_HSV(translator));
                case CS_XYZ:
                        return(RGB_to_XYZ(translator));
		case CS_LAB:
			return(RGB_to_LAB(translator));
                default:
                        assert (1 != 1);
        }
}
