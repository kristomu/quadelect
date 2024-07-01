/* zmg.h:  Ziggurat Method Generator of Zero-Mean Gaussians
 *
 * Copyright 2019 Frank R. Kschischang <frank@ece.utoronto.ca>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about ZMG, visit
 *
 *     http://www.comm.utoronto.ca/frank/ZMG
 */


/*
   This program requires a source of pseudorandom, independent,
   uniformly-distributed, 32-bit unsigned integers, such as provided
   by Melissa E. O'Neill's PCG family; see www.pcg-random.org.

   In the following, RNG = random number generator.

   The assumed interface to the RNG is a function with prototype
     uint32_t ZMG_U32(ZMG_STATE *rng_p);
   where
     ZMG_U32 is the name of the RNG function, and
     ZMG_STATE is an RNG-implementation-specific data type.

   Seeding the RNG is accomplished by calling
     (void)ZMG_SEED(ZMG_STATE *rng_p, ZMG_SEED_t seed);

   These names are defined below.  We assume the caller of zmgf and zmgd
   will provide rng_p, which should point to a properly initialized
   data structure.
*/

#include "pcg_variants.h"
typedef pcg32f_random_t ZMG_STATE;
typedef uint64_t ZMG_SEED_t;
#define ZMG_U32 pcg32f_random_r
#define ZMG_SEED pcg32f_srandom_r

/*
   These are the ZMG functions.
*/

float zmgf(ZMG_STATE *);
double zmgd(ZMG_STATE *,ZMG_STATE *);
void seedzmgf(ZMG_STATE *);
void seedzmgd(ZMG_STATE *,ZMG_STATE *);
