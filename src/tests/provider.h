#pragma once

#include "strategy/strategies.h"

class test_provider {
	private:
		std::vector<std::shared_ptr<criterion_test> > all_tests;
		std::map<std::string,
			std::vector<std::shared_ptr<criterion_test> > >
			tests_by_category;
		std::map<std::string, std::shared_ptr<criterion_test> >
		tests_by_name;

	public:
		test_provider();

		std::vector<std::shared_ptr<criterion_test> >
		get_all_tests() const {
			return all_tests;
		}

		std::shared_ptr<criterion_test>
		get_test_by_name(std::string name) const;

		std::vector<std::shared_ptr<criterion_test> >
		get_tests_by_category(std::string category_name) const;
};