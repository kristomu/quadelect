#include <string>
#include <iostream>

#include "types.h"

using namespace std;

// Margins calculation types (wv, margins, lv, etc). Let's try using a
// Strategy pattern - a base class for these as well as inherited classes for
// the different strategies.

// ------------------------------------------------------------------------ //
// Strategy context and factory code.

void pairwise_type::dealloc() {
	if (kind != NULL)
		delete kind;
	kind = NULL;
}

void pairwise_type::copy(const pairwise_type & source) {
	if (source.kind != NULL) {
		pairwise_ident p = source.get();
		set(p);
	} else
		kind = NULL;
}

pairwise_strategy * pairwise_type::provide_strategy(pairwise_ident id) {

	switch(id) {
		case CM_WV: return(new pws_wv);
		case CM_LV: return(new pws_lv);
		case CM_MARGINS: return(new pws_margins);
		case CM_LMARGINS: return(new pws_lmargins);
		case CM_PAIRWISE_OPP: return(new pws_pairwise_opp);
		case CM_WTV: return(new pws_wtv);
		case CM_TOURN_WV: return(new pws_tourn_wv);
		case CM_TOURN_SYM: return(new pws_tourn_sym);
		case CM_FRACTIONAL_WV: return(new pws_fractional_wv);
		case CM_RELATIVE_MARGINS: return(new pws_rel_margins);
		case CM_KEENER_MARGINS: return(new pws_keener_margins);
		default: return(NULL);
	}
}

void pairwise_type::set(const pairwise_ident & in) {
	dealloc();
	kind = provide_strategy(in);
}

// Pairwise producer, whose purpose is to list every possible strategy.

list<pairwise_type> pairwise_producer::provide_all_strategies() const {

	list<pairwise_type> output;

	// This is what CM_FIRST and CM_LAST is for!

	int p;

	for (p = CM_FIRST; p <= CM_LAST; ++p)
		output.push_back(pairwise_type((pairwise_ident)p));

	return(output);
}
