#ifndef _INTERP_ALL
#define _INTERP_ALL

#include "interpreter.h"
#include "rank_order.h"

#include <list>

std::list<interpreter *> get_all_interpreters() {

	std::list<interpreter *> out;

	out.push_back(new rank_order_int());

	return (out);
}

#endif
