#include "provider.h"

test_provider::test_provider() {
	all_tests = {
		std::make_shared<burial>(),
		std::make_shared<compromising>(),
		std::make_shared<two_sided_strat>(),
		std::make_shared<two_sided_reverse>(),
		std::make_shared<coalitional_strategy>()
	};

	for (auto & test_ptr: all_tests) {
		tests_by_name[test_ptr->name()] = test_ptr;
		tests_by_category[test_ptr->category()].push_back(test_ptr);
	}
}

std::shared_ptr<criterion_test>
test_provider::get_test_by_name(std::string name) const {

	return tests_by_name.find(name)->second;
}

std::vector<std::shared_ptr<criterion_test> >
test_provider::get_tests_by_category(std::string
	category_name) const {

	return tests_by_category.find(category_name)->second;

}