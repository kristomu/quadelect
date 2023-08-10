#pragma once

#include "interpreter.h"
#include "rank_order.h"

#include <list>

std::vector<std::shared_ptr<interpreter> > get_all_interpreters() {

	std::vector<std::shared_ptr<interpreter> > out;

	out.push_back(std::make_shared<rank_order_int>());

	return (out);
}