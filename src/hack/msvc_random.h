#pragma once

// MSVC/Windows HACKS. Windows doesn't implement the Linux random
// functions. I shouldn't be using them, either; everything should
// go through the custom random number generator classes. So this
// indicates a deeper problem that should be fixed. But to get it
// to compile in the meantime, this hack aliases the functions we
// need in a way that MSVC understands.

// MSVC RANDOM HACK
#if _WIN32
#define srand48(x) (srand((unsigned int)(x)))
#define drand48() ((double)rand()/(RAND_MAX+1.0))
#define srandom(x) (srand((unsigned int)(x)))
#define random() (rand())
#endif