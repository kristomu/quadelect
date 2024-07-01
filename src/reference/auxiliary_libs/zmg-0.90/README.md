# ZMG: Ziggurat Method Generator of Zero-Mean Gaussians

[the ZMG website]: http://www.comm.utoronto.ca/frank/ZMG
[PCG-Random website]: http://www.pcg-random.org

This software is an implementation of a version of the modified ziggurat
algorithm of McFarland for generating Gaussian pseudorandom variates.  Some
use is made of type-punning, taking advantage of the IEEE 754 floating point
representation, gaining speed at the expense of portability.  The current
implementation runs on 32- or 64-bit little endian platforms (e.g., machines
compatible with the Intel x86 architecture), but can be easily modified to
run on other platforms.  Though compatible with any source of 32-bit
unsigned pseudorandom integers, this implementation makes use of O'Neill's
PCG family of random number generators, available at [PCG-Random website].

Full details can be found at [the ZMG website].  Two Gaussian pseudorandom
variate generators (one returning float, the other return double) are
provided, along with corresponding seed functions.  For most applications,
the float version should provide sufficient randomness (and it runs about
twice as fast).

## Documentation and Examples

Visit [the ZMG website] for information on how to use these programs, and
read the file `guide.md`.  Working sample code can be found in the `test`
subdirectory supplied with ZMG.

## Building

These functions rely on O'Neill's PCG family of random number generators.
Navigate to [PCG-Random website], and download the C implementation (*not* the
minimal C implementation).   After unpacking, simply copy the file
`include/pcg_variants.h` into the working directory here (where you should
also find `zmg.h`).

The functions are written in C.  As noted above, due to the use of
type-punning, there is indeed a significant platform dependency.  On a
Unix-style system (e.g., Linux), assuming you have gcc installed,
you should be able to just type

    make

## Testing

Type

    cd test; make

This will build single-precision (suffix f) and double-precsion (suffix d)
versions of programs to generate moments, histograms, or simply just a large
number number of random variates (for speed-testing).

## Important Files

* `guide.md`: user's guide
* `zmg.h`: the include file defining four ZMG functions
* `zmgf.c`: the single-precision implementation
* `zmgd.c`: the double precision implementation
* `pcg_variant.h`: don't forget to download this from [PCG-Random website]

## Subdirectories

* `report`: the technical report describing how these functions work
* `test`: source files for testing
* `extras`: a wolframscript for generating the required look-up tables
