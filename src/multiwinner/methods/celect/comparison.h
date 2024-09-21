// comparison function; more of a functional class.

#ifndef __COMPARISON
#define __COMPARISON

#include <vector>
#include <string>

// Components:
//	preinit (used to set quotas for CPO-STV and such; should be called on
//		constructor).
//	set_data (used to set data references, e.g ballot box for CPO-STV).
//	compare (does the thing)
//	get_first_score, get_second_score (gets victory (wv) score for A and B
//	respectively).
//	get_identity (get name of the comparison method). (should be a class
//	Identifiable)

// TODO: Extensibility, e.g differing quotas for CPO-STV.

template<typename T>
class ComparisonMethod {

	protected:
		// The private mechanics of the inherited stuff doesn't concern us,
		// therefore this is commented out. Consider it a guide.
		/*	virtual void set_data(int candidates_in, const T & data_in);
			virtual void post_initialize(); // set quotas etc*/

	public:
		virtual void init(int candidates_in, const T & data_in) = 0;
		// set to constrain to whatever data the method requires.
		// just make this alias so we don't have to cutnpaste.
		//ComparisonMethod(int candidates_in, const T & data_in);

		virtual void compare(const std::vector<bool> & first_set,
			const std::vector<bool> & second_set) = 0;
		virtual double get_first_score() = 0;
		virtual double get_second_score() = 0;
		virtual std::string get_identity(bool long_form) = 0;
};

#endif
