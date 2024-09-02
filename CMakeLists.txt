cmake_minimum_required (VERSION 3.1)

project(quadelect)

set (CMAKE_CXX_STANDARD 14)
# Disable MSVC's /Od. This is kind of a HACK, fix later.
set (CMAKE_BUILD_TYPE Release)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Parameters
# Consider splitting into debug and release (but by how slow things are
# without optimization, even debug would need the -O switches, or we'd wait
# forever for the bug to trigger).

if (MSVC)
	# I'm not too sure about fp:fast? The docs seem to say that it doesn't
	# invalidate non-finite floating point values, so it appears to be OK?

	# The following warnings have been disabled:

	# Warning C4464: relative paths in include
	#		I should fix these, but I don't want them spamming up
	#		the console before I do.

	# Warning C4100: unused function parameter
	#		Fixing these would clutter up function definitions that
	#		have a common template, e.g. int foo(int bar) would have
	#		to be turned into int foo(int /*bar*/) whenever we don't
	#		use the bar paramete. That's too ugly, so I'm removing this.

	# Warning C4127: conditional expression is constant
	#		Triggered by external libraries and thus not something we
	#		can do anything about.

	# Warning C4244: implicit conversion may lead to loss of data
	#		Mostly size_t/int problems. Fixing them is going to take a
	#		lot of time and I don't want them spamming up the console
	#		in the meantime.

	# The analog of Werror is /WX. Add that when it won't break the
	# build.

	# And even with these parameters set, Linux quadelect is about
	# twice as fast as Windows. Find out why later.
	set(CMAKE_CXX_FLAGS "/O2 /fp:fast /EHsc /W4 /wd4464 /wd4100 /wd4127 /wd4244 /w34265")
else()
	# The unused-parameter warning suggests some refactoring, but is yet too
	# spammy, so I've disabled it. I've also disabled the implict fallthrough
	# warning as it falsely triggers on SpookyHash.
	# -flto may lead to further speedups, but linking with it enabled is way
	# too slow, so it's been omitted here.
	set(CMAKE_CXX_FLAGS "-gdwarf-4 -ggdb -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -Werror -march=native -mtune=native -fno-math-errno -fno-trapping-math -fno-signed-zeros -Wno-unknown-warning-option")
endif()

# g++: Disable the free-nonheap-object warning as it seems to be buggy.
# See e.g. https://stackoverflow.com/questions/75348400
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options("-Wno-free-nonheap-object")
endif()

# clang++: Silence warnings originating in STL headers.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "[Cc]lang")
	add_compile_options("-Wno-delete-non-abstract-non-virtual-dtor")

	# Silence warning that std::random_shuffle is deprecated. These
	# point to a real problem - that we're sometimes using the built-in
	# random number generator, but it's too much work to fix now, much
	# less to do so without changing Yee random seed values.
	# TODO: FIX.
	add_compile_options("-Wno-deprecated-declarations")
endif()

# External library requirements

# vcpkg uses another package for Config++.
if (MSVC)
	find_package(libconfig REQUIRED)
	add_library(config++ ALIAS libconfig::libconfig++)
else()
	find_package(Config++ REQUIRED)

	if (NOT EXISTS "${CONFIG++_INCLUDE_DIR}/libconfig.h++")
		message(FATAL_ERROR "Could not find libconfig++ header file!")
	endif()
endif()

find_package(GLPK REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PNG REQUIRED)
find_package(Boost REQUIRED)

find_library(GETOPT_LIB getopt)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src/ ${PROJECT_SOURCE_DIR}/src/lib/)

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

add_library(qe_singlewinner_methods
	src/coalitions/coalitions.cc
	src/singlewinner/get_methods.cc
	src/singlewinner/acp/acp.cc
	src/singlewinner/acp/general_acp.cc
	src/singlewinner/brute_force/brute.cc
	src/singlewinner/brute_force/bruterpn.cc
	src/singlewinner/brute_force/rpn/custom_func.cc
	src/singlewinner/cardinal/asm.cc
	src/singlewinner/cardinal/cumulative.cc
	src/singlewinner/contingent/donation.cc
	src/singlewinner/desc_coalitions/desc_coalition.cc
	src/singlewinner/dmt/fpa_fpc.cc
	src/singlewinner/dmt/ifpp_like.cc
	src/singlewinner/dmt/strat_ifpp.cc
	src/singlewinner/elimination/auto_runoff.cc
	src/singlewinner/elimination/elimination.cc
	src/singlewinner/experimental/3exp.cc
	src/singlewinner/experimental/beat_chain.cc
	src/singlewinner/experimental/disqelim.cc
	src/singlewinner/experimental/fpa_experiment.cc
	src/singlewinner/experimental/sv2.cc
	src/singlewinner/experimental/takedown.cc
	src/singlewinner/experimental/tup.cc
	src/singlewinner/experimental/rmr1.cc
	src/singlewinner/fpc.cc
	src/singlewinner/gradual_c_b.cc
	src/singlewinner/meta/chain_climbing.cc
	src/singlewinner/meta/benham.cc
	src/singlewinner/meta/comma.cc
	src/singlewinner/meta/filters/clamp.cc
	src/singlewinner/meta/filters/mean_utility.cc
	src/singlewinner/meta/filters/meanutil_trunc.cc
	src/singlewinner/meta/filters/normalize.cc
	src/singlewinner/meta/slash.cc
	src/singlewinner/method.cc
	src/singlewinner/no_elimination_irv.cc
	src/singlewinner/pairwise/dodgson_approxs.cc
	src/singlewinner/pairwise/keener.cc
	src/singlewinner/pairwise/kemeny.cc
	src/singlewinner/pairwise/least_rev.cc
	src/singlewinner/pairwise/method.cc
	src/singlewinner/pairwise/minmaxy_exper.cc
	src/singlewinner/pairwise/odm/hits.cc
	src/singlewinner/pairwise/odm/odm_atan.cc
	src/singlewinner/pairwise/odm/odm.cc
	src/singlewinner/pairwise/odm/odm_gen.cc
	src/singlewinner/pairwise/odm/odm_tanh.cc
	src/singlewinner/pairwise/random/randpair.cc
	src/singlewinner/pairwise/rpairs.cc
	src/singlewinner/pairwise/simple_methods.cc
	src/singlewinner/pairwise/sinkhorn.cc
	src/singlewinner/pairwise/tournament.cc
	src/singlewinner/positional/aggregator.cc
	src/singlewinner/positional/positional.cc
	src/singlewinner/positional/simple_methods.cc
	src/singlewinner/quick_runoff.cc
	src/singlewinner/random/randball.cc
	src/singlewinner/random/randcand.cc
	src/singlewinner/sets/condorcet.cc
	src/singlewinner/sets/debug/venzke_landau.cc
	src/singlewinner/sets/dmt.cc
	src/singlewinner/sets/inner_burial.cc
	src/singlewinner/sets/max_elements/det_sets.cc
	src/singlewinner/sets/max_elements/pdom.cc
	src/singlewinner/sets/max_elements/sdom.cc
	src/singlewinner/sets/max_elements/smith.cc
	src/singlewinner/sets/mdd.cc
	src/singlewinner/sets/mutual_majority.cc
	src/singlewinner/sets/partition.cc
	src/singlewinner/sets/topological.cc
	src/singlewinner/stats/cardinal.cc
	src/singlewinner/stats/mode.cc
	src/singlewinner/stats/var_median/vmedian.cc
	src/singlewinner/tools/first_seen.cc
	src/singlewinner/young.cc)

# Includes Young and Kemeny, which need GLPK.
target_link_libraries(qe_singlewinner_methods ${GLPK_LIBRARIES})

add_library(qe_multiwinner_methods
	src/multiwinner/auction.cc
	src/multiwinner/exhaustive/scored_method.cc
	src/multiwinner/helper/errors.cc
	src/multiwinner/compat_qbuck.cc
	src/multiwinner/dhwl.cc
	src/multiwinner/dhwl_mat.cc
	src/multiwinner/meek_stv.cc
	src/multiwinner/methods.cc
	src/multiwinner/prbucklin.cc
	src/multiwinner/psc.cc
	src/multiwinner/quotas.cc
	src/multiwinner/qbuck.cc
	src/multiwinner/qpq.cc
	src/multiwinner/qrange_stv.cc
	src/multiwinner/randballots.cc
	src/multiwinner/range_stv.cc
	src/multiwinner/rusty/aux/dsc.cc
	src/multiwinner/shuntsstv.cc
	src/multiwinner/stv.cc
	src/stats/multiwinner/mwstats.cc)

add_library(quadelect_lib src/common/ballots.cc
	src/bandit/lilucb.cc
	src/common/cache.cc
	src/distances/vivaldi_test.cc
	src/generator/ballotgen.cc
	src/generator/iac.cc
	src/generator/impartial_gen.cc
	src/generator/spatial/gaussian.cc
	src/generator/spatial/spatial.cc
	src/generator/spatial/uniform.cc
	src/grad_median/grad_median.cc
	src/images/color/color.cc
	src/interpreter/rank_order.cc
	src/lib/spookyhash/SpookyV2.cpp
	src/linear_model/constraints/constraint_polytope.cc
	src/linear_model/constraints/constraint_tools.cc
	src/linear_model/constraints/direct_relative_criterion.cc
	src/linear_model/constraints/numvoters.cc
	src/linear_model/constraints/pairwise.cc
	src/linear_model/constraints/relative_criteria/clones.cc
	src/linear_model/constraints/relative_criteria/elimination.cc
	src/linear_model/constraints/relative_criteria/isda.cc
	src/linear_model/constraints/relative_criteria/mono-raise.cc
	src/linear_model/constraints/relative_criteria/strong_um.cc
	src/linear_model/constraints/relative_criterion_producer.cc
	src/linear_model/lin_relation/constraint_set.cc
	src/linear_model/lin_relation/lin_relation.cc
	src/linear_model/polytope/equality_polytope.cc
	src/linear_model/polytope/polytope.cc
	src/linear_model/polytope/properties/bounding_box.cc
	src/linear_model/polytope/properties/center.cc
	src/linear_model/polytope/properties/distance.cc
	src/modes/barycentric.cc
	src/modes/breg.cc
	src/modes/interpret.cc
	src/modes/yee.cc
	src/output/png_writer.cc
	src/pairwise/abstract_matrix.cc
	src/pairwise/beatpath.cc
	src/pairwise/cache_matrix.cc
	src/pairwise/grad_matrix.cc
	src/pairwise/matrix.cc
	src/pairwise/types.cc
	src/random/random.cc
	src/reference_tests/engine/twotest.cc
	src/reference_tests/tests/monotonicity/mono_add.cc
	src/reference_tests/tests/monotonicity/mono_append.cc
	src/reference_tests/tests/monotonicity/mono_raise.cc
	src/reference_tests/tests/monotonicity/mono_sub.cc
	src/reference_tests/tests/monotonicity/monotonicity.cc
	src/reference_tests/tests/rev_symmetry.cc
	src/reference_tests/two_tests.cc
	src/singlewinner/brute_force/rpn/chaotic_functions.cc
	src/singlewinner/stats/cardinal.cc
	src/simulator/bernoulli.cc
	src/simulator/runtime.cc
	src/simulator/simulator.cc
	src/simulator/utility/opt_frequency.cc
	src/simulator/utility/vse.cc
	src/simulator/stubs/bernoulli.cc
	src/stats/confidence/as241.cc
	src/stats/confidence/confidence.cc
	src/stats/coordinate_gen.cc
	src/stats/distributions/gaussian.cc
	src/stats/quasirandom/r_sequence.cc
	src/tests/manual/dh2.cc
	src/tests/manual/neutral_2cddt.cc
	src/tests/provider.cc
	src/tests/runner.cc
	src/tests/strategy/ballots_by_support.cc
	src/tests/strategy/strategies.cc
	src/tests/test.cc
	src/tests/quick_dirty/monotonicity.cc
	src/tools/ballot_tools.cc
	src/tools/cp_tools.cc
	src/tools/factoradic.cc
	src/tools/time_tools.cc
	src/singlewinner/sets/inner_burial.cc
	src/tools/tools.cc)

# Required by the Yee mode.
target_link_libraries(quadelect_lib ${PNG_LIBRARIES})

# Set the different executables.
# They might not all need all the files above, but I'll fix that later.

add_executable(qe_verifier src/singlewinner/brute_force/general_rpn/verifier/verifier.cc)
target_link_libraries(qe_verifier qe_rpn_search quadelect_lib config++)
target_link_libraries(qe_verifier ${GLPK_LIBRARIES})

add_executable(qe_compositor src/singlewinner/brute_force/general_rpn/composition/polytope_compositor.cc)
target_link_libraries(qe_compositor qe_rpn_search qe_singlewinner_methods quadelect_lib config++)

add_executable(qe_sifter src/singlewinner/brute_force/general_rpn/sifter.cc)
target_link_libraries(qe_sifter qe_rpn_search quadelect_lib)

add_executable(qe_invariance_sifter src/singlewinner/brute_force/general_rpn/invariance.cc)
target_link_libraries(qe_invariance_sifter qe_rpn_search quadelect_lib)

add_executable(qe_find_distinguisher src/main/find_distinguisher.cc)
target_link_libraries(qe_find_distinguisher qe_singlewinner_methods quadelect_lib)

# TODO: Better name and description.
add_executable(brute_detail src/main/test_strategy_brute_detail.cc)
target_link_libraries(brute_detail qe_singlewinner_methods quadelect_lib)

add_executable(bandit_search src/main/test_strategy_linbrute_bandit.cc)
target_link_libraries(bandit_search qe_singlewinner_methods quadelect_lib)

add_executable(bandit_monotonicity src/main/test_monotonicity_bandit.cc)
target_link_libraries(bandit_monotonicity qe_singlewinner_methods quadelect_lib)

add_executable(fixed_strategy src/main/test_strategy_fixed.cc)
target_link_libraries(fixed_strategy qe_singlewinner_methods quadelect_lib)

add_executable(criterion_test src/main/ttetest_cache.cc)
target_link_libraries(criterion_test qe_singlewinner_methods quadelect_lib)

add_executable(quadelect src/main/combined.cc)
target_link_libraries(quadelect qe_singlewinner_methods quadelect_lib)

# Getopt needs to be declared separately on Windows.
if (MSVC)
	target_link_libraries(quadelect ${GETOPT_LIB})
endif()

add_executable(qe_dim src/main/dimensionality.cc)
target_link_libraries(qe_dim qe_singlewinner_methods quadelect_lib)

add_executable(monotonator src/main/monotonator.cc)
target_link_libraries(monotonator qe_singlewinner_methods quadelect_lib)

add_executable(vse_brute src/main/test_vse_brute.cc)
target_link_libraries(vse_brute qe_singlewinner_methods quadelect_lib)

add_executable(vse_bandit src/main/test_vse_bandit.cc)
target_link_libraries(vse_bandit qe_singlewinner_methods quadelect_lib)

add_executable(test_bandit_correctness src/main/test_bandit_correctness.cc)
target_link_libraries(test_bandit_correctness qe_singlewinner_methods quadelect_lib)

add_executable(multiwinner src/main/multiwinner.cc)
target_link_libraries(multiwinner qe_multiwinner_methods qe_singlewinner_methods quadelect_lib)

add_executable(yee_reference_benchmark src/reference/yee/yee_borda.cc)
add_executable(vse_reference src/reference/vse/vse.cc)
