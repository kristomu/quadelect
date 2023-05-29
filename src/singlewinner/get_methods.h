#pragma once

#include "all.h"

#include <list>

std::list<pairwise_method *> get_pairwise_methods(
	const list<pairwise_type> & types, bool include_experimental);

std::list<positional *> get_positional_methods(bool truncate);

std::list<pairwise_method *> get_sets();

template <typename T, typename Q> std::list<election_method *> expand_meta(
	const std::list<T *> & base_methods, const std::list<Q *> & sets,
	bool elimination_works) {

	list<election_method *> kombinat;
	typename list<Q *>::const_iterator spos;

	for (typename list<T *>::const_iterator pos = base_methods.begin();
		pos != base_methods.end(); ++pos) {
		for (spos = sets.begin(); spos != sets.end(); ++spos) {
			if ((*pos)->name() == (*spos)->name()) {
				continue;
			}

			kombinat.push_back(new comma(*pos, *spos));
			// Use indiscriminately at your own risk! I'm not
			// trusting this wholly until I can do hopefuls
			// transparently.
			if (elimination_works) {
				kombinat.push_back(new slash(*pos, *spos));
			}
		}
		// These are therefore constrained to positional
		// methods for the time being. I think there may be bugs with
		// hopefuls for some of the advanced methods. I'm therefore
		// not doing anything with them, limiting LE and slash to
		if (elimination_works) {
			kombinat.push_back(new loser_elimination(*pos, true,
					true));
			kombinat.push_back(new loser_elimination(*pos, false,
					true));
		}
	}

	return (kombinat);
}

std::list<election_method *> get_singlewinner_methods(bool truncate,
	bool include_experimental);