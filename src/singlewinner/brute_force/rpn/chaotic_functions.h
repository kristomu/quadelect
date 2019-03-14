// Chaotic functions used as unary functions in RPN, to (hopefully) emulate
// some of the weirdness going on in functions that use double precision
// roundoff error, and/or to investigate hunches about "methods that act
// as close to random as we can get without violating certain criteria".

#include "../../../interval/interval.h"

#pragma once

double triangle_wave(double x);
Interval triangle_wave(Interval x);

// Generalized Blancmange function, see
// https://en.wikipedia.org/w/index.php?title=Blancmange_curve&oldid=824538531

double blancmange(double w, double x);
Interval blancmange(double w, Interval x);

/* Minkowski's question-mark function */
// https://en.wikipedia.org/w/index.php?title=Minkowski%27s_question-mark_function&oldid=824984747
double minkowski_q(double x);
Interval minkowski_q(Interval x);