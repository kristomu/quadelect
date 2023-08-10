#pragma once

#include <vector>
#include <string>
#include <math.h>


// Margins calculation types (wv, margins, lv, etc). Let's try using a
// Strategy pattern - a base class for these as well as inherited classes for
// the different strategies.

// The strategies are:
// WV: winning votes
//      A's strength over B is equal to the number of voters preferring A to
//              B if that's stronger than the ones preferring B to A, otherwise
//              0.
// LV: losing votes:
//      A's strength over B is eq to voters not preferring B to A if that's
//              stronger than the ones not preferring A to B, otherwise 0.
// MARGINS: margins:
//      A's strength over B is eq to max of 0 and voters preferring A to B minus
//              voters preferring B to A.
// LMARGINS: linear margins:
//      Like margins, except negative numbers are permitted instead of clamped
//              to zero.
// PAIRWISE_OPP: pairwise opposition:
//      A's strength over B is eq to the number of voters preferring A to B.

// WTV: winning/tying votes
//      Same as WV, but ties also count.
// TOURN_WV: 1 for a win, 0 for everything else.
// TOURN_SYM: 1 for a win, 0 for a tie, -1 for a loss (for "symmetric").

// FRACTIONAL_WV: (winner)/(winner + loser). Only submit if > 1, same as
// wv.
// RELATIVE_MARGINS:
//      Relative margins (a - b) / (a + b).

// KEENER_MARGINS:
//		h ((a + 1) / (a + b + 2)) where h is
//			h(x) = 0.5 + 0.5 sign(x - 0.5) * sqrt(|2x - 1).
//		See meyer.math.ncsu.edu/Meyer/Talks/OD_RankingCharleston.pdf

enum pairwise_ident { CM_FIRST = 0,
	CM_WV = 0, CM_LV = 1, CM_MARGINS = 2, CM_LMARGINS = 3,
	CM_PAIRWISE_OPP = 4, CM_WTV = 5, CM_TOURN_WV = 6, CM_TOURN_SYM = 7,
	CM_FRACTIONAL_WV = 8, CM_RELATIVE_MARGINS = 9, CM_KEENER_MARGINS = 10,
	CM_LAST = 10
};

class pairwise_strategy {
	public:
		virtual pairwise_ident get() const = 0;
		virtual std::string explain() const = 0;

		virtual double transform(double favor, double oppose,
			double num_voters) const = 0;

		virtual ~pairwise_strategy() {}
};

// ------------------------------------------------------------------------ //
// Concrete strategies follow.

class pws_wv : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_WV);
		}
		std::string explain() const {
			return ("wv");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			if (favor > oppose) {
				return (favor);
			} else	{
				return (0);
			}
		}
};

class pws_lv : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_LV);
		}
		std::string explain() const {
			return ("lv");
		}

		double transform(double favor, double oppose,
			double num_voters) const {
			if (num_voters - oppose > num_voters - favor) {
				return (num_voters - oppose);
			} else	{
				return (0);
			}
		}
};

class pws_margins : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_MARGINS);
		}
		std::string explain() const {
			return ("margins");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			return (std::max(0.0, favor - oppose));
		}
};

class pws_lmargins : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_LMARGINS);
		}
		std::string explain() const {
			return ("l-margins");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			return (favor - oppose);
		}
};

class pws_pairwise_opp : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_PAIRWISE_OPP);
		}
		std::string explain() const {
			return ("PO");
		}

		double transform(double favor, double /*oppose*/,
			double /*num_voters*/) const {
			return (favor);
		}
};

class pws_wtv : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_WTV);
		}
		std::string explain() const {
			return ("w/tv");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			if (favor >= oppose) {
				return (favor);
			} else	{
				return (0);
			}
		}
};

class pws_tourn_wv : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_TOURN_WV);
		}
		std::string explain() const {
			return ("tourn-wv");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			if (favor > oppose) {
				return (1);
			} else	{
				return (0);
			}
		}
};

class pws_tourn_sym : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_TOURN_SYM);
		}
		std::string explain() const {
			return ("tourn-sym");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			if (favor > oppose) {
				return (1);
			}
			if (favor == oppose) {
				return (0);
			} else	{
				return (-1);
			}
		}
};

class pws_fractional_wv : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_FRACTIONAL_WV);
		}
		std::string explain() const {
			return ("fwv");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			if (favor > oppose) {
				return (favor / (favor + oppose));
			} else	{
				return (0);
			}
		}
};

class pws_rel_margins : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_RELATIVE_MARGINS);
		}
		std::string explain() const {
			return ("rel-margins");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			return (std::max(0.0, (favor-oppose)/(favor+oppose)));
		}
};

class pws_keener_margins : public pairwise_strategy {
	public:
		pairwise_ident get() const {
			return (CM_KEENER_MARGINS);
		}
		std::string explain() const {
			return ("keener");
		}

		double transform(double favor, double oppose,
			double /*num_voters*/) const {
			double inner = (favor + 1.0) / (favor + oppose + 2.0);
			double outer = 0.5 + copysign(0.5, inner - 0.5) *
				sqrt(fabs(2*inner - 1));

			return (outer);
		}
};

// ------------------------------------------------------------------------ //
// Strategy context and factory follows.

class pairwise_type {
	private:
		pairwise_strategy * kind;

		void dealloc();
		void copy(const pairwise_type & source);
		pairwise_strategy * provide_strategy(pairwise_ident id);

	public:
		pairwise_ident get() const {
			return (kind->get());
		}
		void set(const pairwise_ident & in);
		std::string explain() const {
			return (kind->explain());
		}
		double transform(double favor, double oppose,
			double num_voters) const {
			return (kind->transform(favor, oppose, num_voters));
		}

		// We need to make sure the pointer is *never* exposed to the
		// outside. Otherwise, it would be impossible to know whether
		// we could safely deallocate it or not.

		pairwise_type() {
			kind = NULL;
		}
		pairwise_type(const pairwise_type & source) {
			kind = NULL;
			copy(source);
		}
		void operator=(const pairwise_type & source) {
			copy(source);
		}
		~pairwise_type() {
			dealloc();
		}
		pairwise_type(const pairwise_ident in) {
			kind = provide_strategy(in);
		}
};

class pairwise_producer {
	public:
		std::vector<pairwise_type> provide_all_strategies() const;
};