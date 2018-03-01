# I don't know how these things really work, so here's a simple non-recursive
# test.

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

#We need C++0x for unordered maps and sets (cache).
#AM_CXXFLAGS = -std=c++0x

bin_PROGRAMS = qelect
qelect_SOURCES = src/ballot_tools.h src/ballots.h src/cache.h src/choose_inc.h \
	src/distances/vivaldi_test.h src/generator/ballotgen.h \
	src/generator/dirichlet.h src/generator/spatial/gaussian.h \
	src/generator/impartial.h src/generator/spatial/spatial.h \
	src/generator/spatial/uniform.h \
	src/grad_median/grad_median.h src/pairwise/abstract_matrix.h \
	src/pairwise/beatpath.h src/pairwise/cache_matrix.h \
	src/pairwise/grad_matrix.h src/pairwise/matrix.h src/pairwise/types.h \
	src/random/random.h src/singlewinner/elimination.h \
	src/singlewinner/gradual_c_b.h src/singlewinner/meta/comma.h \
	src/singlewinner/meta/slash.h src/singlewinner/method.h \
	src/singlewinner/pairwise/dodgson_approxs.h \
	src/singlewinner/pairwise/keener.h src/singlewinner/pairwise/kemeny.h \
	src/singlewinner/pairwise/least_rev.h \
	src/singlewinner/pairwise/method.h src/singlewinner/pairwise/odm/hits.h\
	src/singlewinner/pairwise/odm/odm.h \
	src/singlewinner/pairwise/odm/odm_atan.h \
	src/singlewinner/pairwise/odm/odm_gen.h \
	src/singlewinner/pairwise/random/randpair.h \
	src/singlewinner/pairwise/rpairs.h \
	src/singlewinner/pairwise/simple_methods.h \
	src/singlewinner/pairwise/sinkhorn.h \
	src/singlewinner/pairwise/tournament.h \
	src/singlewinner/positional/aggregator.h \
	src/singlewinner/positional/as241.h \
	src/singlewinner/positional/positional.h \
	src/singlewinner/positional/simple_methods.h \
	src/singlewinner/positional/types.h \
	src/singlewinner/random/randball.h \
	src/singlewinner/random/randcand.h \
	src/singlewinner/sets/condorcet.h \
	src/singlewinner/sets/max_elements/c_tt.h \
	src/singlewinner/sets/max_elements/det_sets.h \
	src/singlewinner/sets/max_elements/landau.h \
	src/singlewinner/sets/max_elements/schwartz.h \
	src/singlewinner/sets/max_elements/sdom.h \
	src/singlewinner/sets/max_elements/smith.h \
	src/singlewinner/sets/mdd.h src/singlewinner/sets/partition.h \
	src/singlewinner/sets/topological.h src/singlewinner/stats/cardinal.h \
	src/singlewinner/stats/mode.h \
	src/singlewinner/stats/var_median/comparand.h \
	src/singlewinner/stats/var_median/vmedian.h src/singlewinner/young.h \
	src/stats/mwstats.h src/stats/stats.h src/tools.h src/ballot_tools.cc \
	src/ballots.cc src/cache.cc src/choose_inc.cc \
	src/distances/vivaldi_test.cc src/generator/dirichlet.cc \
	src/generator/spatial/gaussian.cc src/generator/impartial.cc \
	src/generator/spatial/spatial.cc src/grad_median/grad_median.cc \
	src/generator/spatial/uniform.cc \
	src/main/combined.cc \
	src/pairwise/abstract_matrix.cc src/pairwise/beatpath.cc \
	src/pairwise/cache_matrix.cc src/pairwise/grad_matrix.cc \
	src/pairwise/matrix.cc src/pairwise/types.cc \
	src/pairwise/unfinished/sha.cc src/random/random.cc \
	src/singlewinner/elimination.cc src/singlewinner/gradual_c_b.cc \
	src/singlewinner/meta/comma.cc src/singlewinner/meta/slash.cc \
	src/singlewinner/method.cc \
	src/singlewinner/brute_force/brute.cc \
	src/singlewinner/brute_force/bruterpn.cc \
	src/singlewinner/brute_force/rpn/custom_func.cc \
	src/singlewinner/experimental/3exp.cc \
	src/singlewinner/pairwise/dodgson_approxs.cc\
	src/singlewinner/pairwise/keener.cc src/singlewinner/pairwise/kemeny.cc\
	src/singlewinner/pairwise/least_rev.cc \
	src/singlewinner/pairwise/method.cc \
	src/singlewinner/pairwise/odm/hits.cc \
	src/singlewinner/pairwise/odm/odm.cc \
	src/singlewinner/pairwise/odm/odm_atan.cc \
	src/singlewinner/pairwise/odm/odm_gen.cc \
	src/singlewinner/pairwise/random/randpair.cc \
	src/singlewinner/pairwise/rpairs.cc \
	src/singlewinner/pairwise/simple_methods.cc \
	src/singlewinner/pairwise/sinkhorn.cc \
	src/singlewinner/pairwise/tournament.cc \
	src/singlewinner/positional/aggregator.cc \
	src/singlewinner/positional/positional.cc \
	src/singlewinner/positional/simple_methods.cc \
	src/singlewinner/random/randball.cc \
	src/singlewinner/random/randcand.cc src/singlewinner/sets/condorcet.cc \
	src/singlewinner/sets/max_elements/det_sets.cc \
	src/singlewinner/sets/max_elements/sdom.cc \
	src/singlewinner/sets/max_elements/smith.cc \
	src/singlewinner/sets/mdd.cc src/singlewinner/sets/partition.cc \
	src/singlewinner/sets/topological.cc src/singlewinner/stats/cardinal.cc\
	src/singlewinner/stats/mode.cc \
	src/singlewinner/stats/var_median/vmedian.cc src/singlewinner/young.cc \
	src/tools.cc src/singlewinner/positional/as241.c \
	src/tests/disproof.h src/tests/tests/monotonicity/mono_add.h \
	src/tests/tests/monotonicity/mono_append.h \
	src/tests/tests/monotonicity/mono_raise.h \
	src/tests/tests/monotonicity/mono_sub.h \
	src/tests/tests/monotonicity/monotonicity.h \
	src/tests/tests/rev_symmetry.h src/tests/two_tests.h \
	src/tests/engine/twotest.cc src/tests/tests/monotonicity/mono_add.cc \
	src/tests/tests/monotonicity/mono_append.cc \
	src/tests/tests/monotonicity/mono_raise.cc \
	src/tests/tests/monotonicity/mono_sub.cc \
	src/tests/tests/monotonicity/monotonicity.cc \
	src/tests/tests/rev_symmetry.cc src/tests/two_tests.cc \
	src/generator/ballotgen.cc src/stats/confidence/student.cc \
	src/bandit/bandit.cc src/bandit/lucb.cc \
	src/main/strat_test.cc \
	src/modes/breg.cc src/images/color/color.cc src/modes/yee.cc 

qelect_CXXFLAGS = --std=c++0x -Wall -march=native -ggdb

# HACK HACK. Find out how to determine the name and location of glpk the "right"
# way later. (Same with SSL, which is used for SHA.)

qelect_CXXFLAGS += -lglpk -lssl -lcrypto

dist_noinst_SCRIPTS = autogen.sh
