cmake_minimum_required (VERSION 3.1)

project(quadelect)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Parameters
# Consider splitting into debug and release (but by how slow things are
# without optimization, even debug would need the -O switches)

# The unused-parameter warning suggests some refactoring, but is yet too
# spammy, so I've disabled it. I've also disabled the implict fallthrough
# warning as it falsely triggers on SpookyHash.
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -Werror -march=native -mtune=native -ggdb")

# External library requirements

find_package(OpenSSL REQUIRED)
find_package(GLPK REQUIRED)
find_package(Config++ REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

# Internal functionality groupings. Not all programs need all the
# functionality: in particular, the yee/BR program doesn't need the
# gen_custom_function search and vice versa.

add_library(qe_rpn_search src/singlewinner/brute_force/general_rpn/gen_custom_function.cc
	src/singlewinner/brute_force/general_rpn/composition/scenario.cc
	src/singlewinner/brute_force/general_rpn/composition/equivalences.cc
	src/singlewinner/brute_force/general_rpn/composition/test_generator.cc
	src/singlewinner/brute_force/general_rpn/composition/vector_ballot.cc
	src/singlewinner/brute_force/general_rpn/composition/test_results.cc
	src/singlewinner/brute_force/general_rpn/composition/test_instance_gen.cc
	src/singlewinner/brute_force/general_rpn/composition/groups/test_generator_group.cc
	src/config/general_rpn.cc)

add_library(qe_quick_hash src/spookyhash/SpookyV2.cpp)

add_library(qe_singlewinner_methods src/singlewinner/elimination.cc
	src/singlewinner/gradual_c_b.cc
	src/singlewinner/meta/comma.cc
	src/singlewinner/meta/slash.cc
	src/singlewinner/method.cc
	src/singlewinner/brute_force/brute.cc
	src/singlewinner/brute_force/bruterpn.cc
	src/singlewinner/brute_force/rpn/custom_func.cc
	src/singlewinner/experimental/3exp.cc
	src/singlewinner/experimental/fpa_experiment.cc
	src/singlewinner/experimental/sv2.cc
	src/singlewinner/experimental/tup.cc
	src/singlewinner/pairwise/dodgson_approxs.cc
	src/singlewinner/pairwise/keener.cc
	src/singlewinner/pairwise/kemeny.cc
	src/singlewinner/pairwise/least_rev.cc
	src/singlewinner/pairwise/method.cc
	src/singlewinner/pairwise/odm/hits.cc
	src/singlewinner/pairwise/odm/odm.cc
	src/singlewinner/pairwise/odm/odm_atan.cc
	src/singlewinner/pairwise/odm/odm_tanh.cc
	src/singlewinner/pairwise/odm/odm_gen.cc
	src/singlewinner/pairwise/random/randpair.cc
	src/singlewinner/pairwise/rpairs.cc
	src/singlewinner/pairwise/simple_methods.cc
	src/singlewinner/pairwise/sinkhorn.cc
	src/singlewinner/pairwise/tournament.cc
	src/singlewinner/pairwise/minmaxy_exper.cc
	src/singlewinner/positional/aggregator.cc
	src/singlewinner/positional/positional.cc
	src/singlewinner/positional/simple_methods.cc
	src/singlewinner/random/randball.cc
	src/singlewinner/random/randcand.cc
	src/singlewinner/sets/condorcet.cc
	src/singlewinner/sets/max_elements/det_sets.cc
	src/singlewinner/sets/max_elements/sdom.cc
	src/singlewinner/sets/max_elements/pdom.cc
	src/singlewinner/sets/max_elements/smith.cc
	src/singlewinner/sets/mdd.cc
	src/singlewinner/sets/partition.cc
	src/singlewinner/sets/topological.cc
	src/singlewinner/stats/cardinal.cc)

add_library(quadelect_lib src/tools/tools.cc
	src/interpreter/rank_order.cc
	src/tools/cp_tools.cc
	src/tools/ballot_tools.cc
	src/tools/factoradic.cc
	src/ballots.cc
	src/cache.cc
	src/choose_inc.cc
	src/distances/vivaldi_test.cc
	src/generator/spatial/gaussian.cc
	src/generator/impartial_gen.cc
	src/generator/spatial/spatial.cc
	src/grad_median/grad_median.cc
	src/generator/spatial/uniform.cc
	src/pairwise/abstract_matrix.cc
	src/pairwise/beatpath.cc
	src/pairwise/cache_matrix.cc
	src/pairwise/grad_matrix.cc
	src/pairwise/matrix.cc
	src/pairwise/types.cc
	src/pairwise/unfinished/sha.cc
	src/random/random.cc
	src/singlewinner/brute_force/rpn/chaotic_functions.cc
	src/singlewinner/stats/mode.cc
	src/singlewinner/stats/var_median/vmedian.cc
	src/singlewinner/young.cc
	src/singlewinner/fpc.cc
	src/singlewinner/desc_coalitions/dsc.cc
	src/singlewinner/desc_coalitions/desc_coalition.cc
	src/tests/engine/twotest.cc
	src/tests/tests/monotonicity/mono_add.cc
	src/tests/tests/monotonicity/mono_append.cc
	src/tests/tests/monotonicity/mono_raise.cc
	src/tests/tests/monotonicity/mono_sub.cc
	src/tests/tests/monotonicity/monotonicity.cc
	src/tests/tests/rev_symmetry.cc
	src/tests/two_tests.cc
	src/linear_model/lin_relation/lin_relation.cc
	src/linear_model/lin_relation/constraint_set.cc
	src/linear_model/polytope/equality_polytope.cc
	src/linear_model/polytope/polytope.cc
	src/linear_model/polytope/properties/bounding_box.cc
	src/linear_model/polytope/properties/center.cc
	src/linear_model/polytope/properties/distance.cc
	src/linear_model/constraints/constraint_tools.cc
	src/linear_model/constraints/constraint_polytope.cc
	src/linear_model/constraints/numvoters.cc
	src/linear_model/constraints/pairwise.cc
	src/linear_model/constraints/direct_relative_criterion.cc
	src/linear_model/constraints/relative_criterion_producer.cc
	src/linear_model/constraints/relative_criteria/clones.cc
	src/linear_model/constraints/relative_criteria/mono-raise.cc
	src/linear_model/constraints/relative_criteria/elimination.cc
	src/linear_model/constraints/relative_criteria/isda.cc
	src/generator/ballotgen.cc
	src/stats/confidence/confidence.cc
	src/bandit/bandit.cc
	src/bandit/binomial.cc
	src/bandit/lilucb.cc
	src/main/strat_test.cc
	src/modes/breg.cc
	src/images/color/color.cc
	src/modes/yee.cc
	src/modes/interpret.cc)

# Set the different executables.
# They might not all need all the files above, but I'll fix that later.

add_executable(qe_verifier src/singlewinner/brute_force/general_rpn/verifier/verifier.cc)
target_link_libraries(qe_verifier qe_rpn_search quadelect_lib glpk config++)

add_executable(qe_compositor src/singlewinner/brute_force/general_rpn/composition/polytope_compositor.cc)
target_link_libraries(qe_compositor qe_rpn_search qe_singlewinner_methods quadelect_lib glpk config++)

add_executable(qe_sifter src/singlewinner/brute_force/general_rpn/sifter.cc)
target_link_libraries(qe_sifter qe_rpn_search qe_quick_hash quadelect_lib)

add_executable(qe_invariance_sifter src/singlewinner/brute_force/general_rpn/invariance.cc)
target_link_libraries(qe_invariance_sifter qe_rpn_search qe_quick_hash quadelect_lib)

# TODO: Better name and description.
add_executable(brute_detail src/main/test_strategy_brute_detail.cc)
target_link_libraries(brute_detail qe_singlewinner_methods quadelect_lib)

add_executable(quadelect src/main/combined.cc)
target_link_libraries(quadelect qe_singlewinner_methods quadelect_lib glpk crypto)
