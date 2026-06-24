#pragma once
// Common Schulze STV functions.

// This code is

/*   (c) Markus Schulze, 2007                                                  */
/*   markus.schulze@alumni.tu-berlin.de                                        */
/*   draft, 24 September 2007                                                  */

// unless otherwise noted.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include <vector>

#include "multiwinner/methods/methods.h" // council_t

// This is a bit ugly, but I don't like globals (and the compiler doesn't either).
// Extract out stuff later.

const long double eps1=0.00000000000001;
const long double eps2=0.0000000001;
const long double eps3=0.0000000005;
const long double eps4=0.00000001;

struct ArcElement {
	int start;
	int end;
	long double Value;
	long double Cap;
	int upper;
	int lower;
};

struct VertexElement {
	int  marked;
	int  upper;
	bool direction;
};

struct IndiffElement {
	int Value;
	int upper;
	int lower;
};

struct VotesElement {
	long double Value;
	bool marked;
};

struct DefeatElement {
	int winner;
	int loser;
	long double Value;
	int RN;
	int UN;
};


class SchulzeCommon {
	protected:
		int M;      /* number of seats      */
		int C;      /* number of candidates */
		int N;      /* number of voters     */
		std::vector<int>
		Vote;  /* Vote[C*i+j] is the preference of voter i for candidate j */

		int Comb1;
		int Comb2;
		int *Matrix1;
		long double *Matrix2;
		int *Kombi;

		int    time1,time2;
		time_t start,finish;
		int    elapsed_time;

		int i99;

		int N2;
		std::vector<int> Vote2;
		std::vector<int> Value2;

		int N3;
		unsigned char *Vote3;
		int           *Value3;

		int N4;
		unsigned char *Vote4;
		long double   *Value4;
		int           *Indif4;
		bool          *cool4;

		int           *Kombi4;

		unsigned char *Vote5;
		int           *Value5;

		unsigned char *Vote6;
		long double   *Value6;
		int           *Indif6;
		bool          *cool6;

		int N7;
		bool          *Vote7;
		long double   *Value7;

		int N8;
		bool          *Vote8;
		long double   *Value8;

		unsigned char *Test;
		int           *Test3,*Votes1;

		struct IndiffElement *Indiff,*Indiff7,*Indiff8;
		struct ArcElement    *Arcs,*Arcs7,*Arcs8;
		struct VertexElement *Vertices,*Vertices7,*Vertices8;
		struct VotesElement  *Votes;

		int         g1,g2,g3,m1,Length4,Length6,Length7,LengthArc,LengthVertex;
		long double Output1;

		int SimplexNichtNoetig;

		int LengthIndiff7,LengthIndiff8,LengthArc7,LengthArc8,LengthVertex7,
			LengthVertex8;

		bool *completed;
		int tobecompleted;
		bool SimplexNoetig;

		FILE *datafile;

		std::vector<int> Sets;
		struct DefeatElement *Link;
		int *Zeilenerster,*Spaltenerster;

	public:
		void Print1();
		void Print2();
		void Reading_the_Input();
		void Analyzing_the_Input();
		void PropCompletionSimple();
		void EKarp();
		void EKarpSimple();
		void PropCompletion();
		void VoteMana();
		void Elimination();

		void print3(const council_t & input); // -KM

		// Read ballots into the Schulze STV arrays. Note that
		// every ballot must have weight one. -KM
		void read_ballot_input(const election_t & input, size_t council_size,
			size_t num_candidates);
};