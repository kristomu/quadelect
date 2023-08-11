#pragma once

#include "all.h"

#include <vector>

std::vector<std::shared_ptr<pairwise_method> >  get_pairwise_methods(
	const std::vector<pairwise_type> & types, bool include_experimental);

std::vector<std::shared_ptr<positional> > get_positional_methods(
	bool truncate);

std::vector<std::shared_ptr<pairwise_method> > get_pairwise_sets();
std::vector<std::shared_ptr<election_method> > get_sets();

// This function could stand being split into two or more: one for
// comma and slash combinations, and one for loser elimination and
// Benham.

template <typename T, typename Q>
std::vector<std::shared_ptr<election_method> > expand_meta(
	const std::vector<std::shared_ptr<T> > & base_methods,
	const std::vector<std::shared_ptr<Q> > & sets,
	bool elimination_works) {

	std::vector<std::shared_ptr<election_method> > combined;
	typename std::vector<std::shared_ptr<Q> >::const_iterator spos;

	for (std::shared_ptr<T> method_ptr : base_methods) {

		// Make a Benham-type variant (NOTE: Not an elimination method!)
		auto benham_ptr = std::make_shared<benham_meta>(method_ptr);
		combined.push_back(benham_ptr);

		for (std::shared_ptr<Q> set_ptr : sets) {
			if (set_ptr->name() == method_ptr->name()) {
				continue;
			}

			combined.push_back(std::make_shared<comma>(set_ptr, method_ptr));
			combined.push_back(std::make_shared<comma>(set_ptr, benham_ptr));
			// Use indiscriminately at your own risk! I'm not
			// trusting this wholly until I can do hopefuls
			// transparently.
			if (elimination_works) {
				combined.push_back(std::make_shared<slash>(set_ptr, method_ptr));
				combined.push_back(std::make_shared<slash>(set_ptr, benham_ptr));
			}
		}
		// These are therefore constrained to positional
		// methods for the time being. I think there may be bugs with
		// hopefuls for some of the advanced methods, so beware.
		if (elimination_works) {
			combined.push_back(std::make_shared<loser_elimination>(
					method_ptr, true, true));
			combined.push_back(std::make_shared<loser_elimination>(
					method_ptr, false, true));
		}
	}

	return combined;
}

std::vector<std::shared_ptr<election_method> > get_singlewinner_methods(
	bool truncate,
	bool include_experimental);
