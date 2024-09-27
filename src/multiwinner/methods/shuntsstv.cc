/*   (c) Markus Schulze, 2007                                                  */
/*   markus.schulze@alumni.tu-berlin.de                                        */
/*   draft, 24 September 2007                                                  */
/*                                                                             */
/* This program has been written in Microsoft Visual C++ 5.0.                  */
/*                                                                             */
/* This program calculates the winners according to the Schulze STV method     */
/* with proportional completion, as defined in this series of papers:          */
/*                                                                             */
/*              http://m-schulze.webhop.net/schulze1.pdf                       */
/*              http://m-schulze.webhop.net/schulze2.pdf                       */
/*              http://m-schulze.webhop.net/schulze3.zip                       */

// Added minimal shunt functions to enable this for quadelect usage, although
// it may be too slow. -KM

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "shuntsstv.h"

#include <numeric>
#include <vector>
#include <list>
#include <set>


// This isn't my code, I'm not going to prioritize fixing the warnings here.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wsequence-point"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-result"

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

/*******************************************************************************/

int over(int i1, int i2) {

	/* over(i,j) is the number of combinations */
	/* to choose j elements out of i elements  */

	int i3,i4;

	i3=i1;

	for (i4=1; i4<i2; i4++) {
		i3=(i3*(i1-i4))/(1+i4);
	}

	return i3;
}

/*******************************************************************************/

void Print1() {
	if (C<27) {
		switch (g1) {
			case  0: fprintf(datafile,"A ");
				break;

			case  1: fprintf(datafile,"B ");
				break;

			case  2: fprintf(datafile,"C ");
				break;

			case  3: fprintf(datafile,"D ");
				break;

			case  4: fprintf(datafile,"E ");
				break;

			case  5: fprintf(datafile,"F ");
				break;

			case  6: fprintf(datafile,"G ");
				break;

			case  7: fprintf(datafile,"H ");
				break;

			case  8: fprintf(datafile,"I ");
				break;

			case  9: fprintf(datafile,"J ");
				break;

			case 10: fprintf(datafile,"K ");
				break;

			case 11: fprintf(datafile,"L ");
				break;

			case 12: fprintf(datafile,"M ");
				break;

			case 13: fprintf(datafile,"N ");
				break;

			case 14: fprintf(datafile,"O ");
				break;

			case 15: fprintf(datafile,"P ");
				break;

			case 16: fprintf(datafile,"Q ");
				break;

			case 17: fprintf(datafile,"R ");
				break;

			case 18: fprintf(datafile,"S ");
				break;

			case 19: fprintf(datafile,"T ");
				break;

			case 20: fprintf(datafile,"U ");
				break;

			case 21: fprintf(datafile,"V ");
				break;

			case 22: fprintf(datafile,"W ");
				break;

			case 23: fprintf(datafile,"X ");
				break;

			case 24: fprintf(datafile,"Y ");
				break;

			case 25: fprintf(datafile,"Z ");
				break;
		}
	} else {
		fprintf(datafile,"%d ",g1+1);
	}
}

/*******************************************************************************/

void Print2() {
	if (C<27) {
		switch (g1) {
			case  0: printf("A ");
				break;

			case  1: printf("B ");
				break;

			case  2: printf("C ");
				break;

			case  3: printf("D ");
				break;

			case  4: printf("E ");
				break;

			case  5: printf("F ");
				break;

			case  6: printf("G ");
				break;

			case  7: printf("H ");
				break;

			case  8: printf("I ");
				break;

			case  9: printf("J ");
				break;

			case 10: printf("K ");
				break;

			case 11: printf("L ");
				break;

			case 12: printf("M ");
				break;

			case 13: printf("N ");
				break;

			case 14: printf("O ");
				break;

			case 15: printf("P ");
				break;

			case 16: printf("Q ");
				break;

			case 17: printf("R ");
				break;

			case 18: printf("S ");
				break;

			case 19: printf("T ");
				break;

			case 20: printf("U ");
				break;

			case 21: printf("V ");
				break;

			case 22: printf("W ");
				break;

			case 23: printf("X ");
				break;

			case 24: printf("Y ");
				break;

			case 25: printf("Z ");
				break;
		}
	} else {
		printf("%d ",g1+1);
	}
}

// -KM

void print3(const council_t & input) {

	for (council_t::const_iterator pos = input.begin();
		pos != input.end(); ++pos) {
		std::cout << cand_name(*pos) << " ";
	}
}


/*******************************************************************************/

// KM
// Note that all weights must be unitary.
void read_ballot_input(const election_t & input, size_t council_size,
	size_t num_candidates) {

	// Set the parameters
	M = council_size;	// number of seats
	C = num_candidates;	// number of candidates
	N = input.size();	// number of voters

	Vote = std::vector<int>(N*C + 1, INT_MAX);
	// Vote[C*i + j] is the pref. of voter i for cand j.

	// Now fill the vote array.
	int voter = 0;
	for (election_t::const_iterator pos = input.begin(); pos !=
		input.end(); ++pos) {

		if (pos->get_weight() != 1) {
			throw std::invalid_argument(
				"Schulze STV: weighted votes are not supported");
		}

		// TODO: Handle equal rank.
		int rank_count = 0;
		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos)
			Vote[C*voter + opos->get_candidate_num()] =
				rank_count++;

		++voter;
	}
}

void Reading_the_Input() {
	int i1,i2,i3,i4,i5;
	char c1;
	std::string Problemname;

	std::cout << "Please insert the name of the data file.\n";
	std::cin >> Problemname;

	time(&start);
	time1=clock();

	datafile =fopen(Problemname.c_str(),"r");

	while (!feof(datafile)) {
		c1=fgetc(datafile);
		switch (c1) {
			case 'M': fscanf(datafile,"%d\n",&M);
				break;

			case 'C': fscanf(datafile,"%d\n",&C);
				break;

			case 'N': fscanf(datafile,"%d\n",&N);
				break;

			case 'F': fscanf(datafile,"%d\n",&i5);
				break;

			case 'B': do c1=fgetc(datafile);
				while (c1!='\n');

				Vote.resize(N*C+1);
				Vote.clear();
				//Vote=(int*)calloc(N*C+1,sizeof(int));

				switch (i5) {
					case 1: for (i1=0; i1<N; i1++) {
							i2=C*i1;

							fscanf(datafile,"%d",&i3);

							for (i3=0; i3<C; i3++) {
								i4=i2+i3;
								fscanf(datafile,"%d",&Vote[i4]);
							}

							fscanf(datafile,"\n");
						}

						break;

					case 2: for (i1=0; i1<N; i1++) {
							i2=C*i1;

							fscanf(datafile,"%d",&i3);

							for (i3=0; i3<C; i3++) {
								Vote[i2+i3]=INT_MAX;
							}

							i3=1;

							c1=fgetc(datafile);
							while (c1!='\n') {
								if (c1 >= 'A' && c1 <= 'Z') {
									int offset = c1 - 'A';

									if (Vote[i2+offset]==INT_MAX) {
										Vote[i2+offset]=i3;
									} else {
										for (i4 = 0; i4 < C;
											++i4) {
											if (Vote[i2+i4] > Vote[i2+offset] && Vote[i2+i4] != INT_MAX) {
												Vote[i2 + i4] = Vote[i2 + offset];
											}
										}
									}

									i3++;
								}

								c1=fgetc(datafile);
							}
						}

						break;
				}

				break;

			case 'E': do c1=fgetc(datafile);
				while (c1!='\n');
				break;
		}
	}

	fclose(datafile);
}

/*******************************************************************************/

void Analyzing_the_Input() {
	int i1,i2,i3,i4,i5,i6,i7,i8,i9,i10;
	bool j1;

	Vote2.resize(N*C+1);
	Value2.resize(N+1);

	for (i1=0; i1<C; i1++) {
		Vote2[i1]=Vote[i1];
	}

	Value2[0]=1;

	N2=0;

	for (i1=1; i1<N; i1++) {
		i2=0;
		j1=false;
		while ((i2<=N2) && (j1==false)) {
			j1=true;

			i5=C*i1;
			i6=C*i2;

			for (i3=0   ; (j1==true) && (i3<C-1); i3++)
				for (i4=i3+1; (j1==true) && (i4<C); i4++) {
					i7 =Vote [i5+i3];
					i8 =Vote [i5+i4];
					i9 =Vote2[i6+i3];
					i10=Vote2[i6+i4];

					if (((i7 >i8) && (i9<=i10))
						||
						((i7 <i8) && (i9>=i10))
						||
						((i7==i8) && (i9!=i10))) {
						j1=false;
					}
				}

			if (j1==false) {
				i2++;
			}

			else {
				Value2[i2]=Value2[i2]++;
			}
		}

		if (j1==false) {
			N2++;
			Value2[N2]=1;

			i3=C*i1;
			i4=C*N2;

			for (i2=0; i2<C; i2++) {
				Vote2[i4+i2]=Vote[i3+i2];
			}
		}
	}

	N2++;
}

/*******************************************************************************/

void PropCompletionSimple() {
	int i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11;
	bool j1,j2,j3,j4;
	long double NewValue;
	int totValue;

	j1=false;

	while (j1==false) {
		j1=true;

		i1=0;
		i2=0;

		for (i3=0; i3<N4; i3++)
			if (cool4 [i3]==false)
				if (Indif4[i3]>i1) {
					j1=false;
					i1=Indif4[i3];
					i2=i3;
				}

		if (j1==false) {
			NewValue=Value4[i2];
			//Newcool =cool4 [i2];

			i10=M*i2;
			for (i3=0; i3<M; i3++) {
				Test[i3]=Vote4[i10+i3];
			}

			i4=0;
			for (i3=0; i3<M; i3++)
				if (Test[i3]==2) {
					Kombi4[i4]=i3;
					i4++;
				}

			totValue=0;
			i5=0;

			for (i3=0; i3<N3; i3++) {
				j2=false;

				for (i4=0; (i4<i1) && (j2==false); i4++)
					if (Vote3[M*i3+Kombi4[i4]]!=2) {
						j2=true;
					}

				if (j2==true) {
					totValue=totValue+Value3[i3];

					i6=0;
					j2=false;

					while ((j2==false) && (i6<i5)) {
						j2=true;

						for (i4=0; (i4<i1) && (j2==true); i4++)
							if (Vote3[M*i3+Kombi4[i4]]!=Vote5[i1*i6+i4]) {
								j2=false;
							}

						if (j2==true) {
							Value5[i6]=Value5[i6]+Value3[i3];
						}

						i6++;
					}

					if (j2==false) {
						for (i4=0; i4<i1; i4++) {
							Vote5[i1*i5+i4]=Vote3[M*i3+Kombi4[i4]];
						}

						Value5[i5]=Value3[i3];

						i5++;
					}
				}
			}

			if (i5==0) {
				i5=i1+1;

				totValue=i1+1;

				for (i3=0; i3<i1; i3++) {
					for (i4=0; i4<i1; i4++) {
						Vote5[i1*i3+i4]=2;
					}

					Vote5[i1*i3+i3]=1;

					Value5[i3]=1;
				}

				for (i3=0; i3<i1; i3++) {
					Vote5[i1*i1+i3]=3;
				}

				Value5[i1]=1;
			}

			i7=N4+i5;

			if (i7>Length4) {
				Vote6 =(unsigned char*)calloc(N4*M+1,sizeof(unsigned char));
				Value6=(long double  *)calloc(N4+1,sizeof(long double));
				Indif6=(int          *)calloc(N4+1,sizeof(int));
				cool6 =(bool         *)calloc(N4+1,sizeof(bool));

				for (i3=0; i3<N4; i3++) {
					Value6[i3]=Value4[i3];
					Indif6[i3]=Indif4[i3];
					cool6 [i3]=cool4 [i3];

					i10=M*i3;
					for (i4=0; i4<M; i4++) {
						i11=i10+i4;
						Vote6[i11]=Vote4[i11];
					}
				}

				free(Vote4);
				free(Value4);
				free(Indif4);
				free(cool4);

				Length4=i7;

				Vote4 =(unsigned char*)calloc(Length4*M+1,sizeof(unsigned char));
				Value4=(long double  *)calloc(Length4+1,sizeof(long double));
				Indif4=(int          *)calloc(Length4+1,sizeof(int));
				cool4 =(bool         *)calloc(Length4+1,sizeof(bool));

				for (i3=0; i3<N4; i3++) {
					Value4[i3]=Value6[i3];
					Indif4[i3]=Indif6[i3];
					cool4 [i3]=cool6 [i3];

					i10=M*i3;
					for (i4=0; i4<M; i4++) {
						i11=i10+i4;
						Vote4[i11]=Vote6[i11];
					}
				}

				free(Vote6);
				free(Value6);
				free(Indif6);
				free(cool6);

				free(Test3);
				Test3=(int*)calloc(Length4+1,sizeof(int));
			}

			i7=N4-1;
			Value4[i2]=Value4[i7];
			cool4 [i2]=cool4 [i7];
			Indif4[i2]=Indif4[i7];

			i6=M*i2;
			i7=M*i7;
			for (i3=0; i3<M; i3++) {
				Vote4[i6+i3]=Vote4[i7+i3];
			}

			i7=N4-1;
			j3=false;

			for (i9=0; i9<N4-1; i9++) {
				j4=true;

				for (i3=0; (i3<M) && (j4==true); i3++)
					if (Test[i3]!=2)
						if (Test[i3]!=Vote4[M*i9+i3]) {
							j4=false;
						}

				if (j4==true) {
					if (j3==false) {
						j3=true;
						i7=i9;
						i6=i9;
					} else {
						Test3[i6]=i9;
						i6=i9;
					}
				}
			}

			if (j3==true) {
				Test3[i6]=N4-1;
			}

			i4=N4-1;

			for (i9=0; i9<i5; i9++) {
				for (i3=0; i3<i1; i3++) {
					Test[Kombi4[i3]]=Vote5[i1*i9+i3];
				}

				i8=0;
				j4=false;

				for (i3=0; i3<M; i3++) {
					if (Test[i3]==2) {
						i8++;
					} else {
						if (Test[i3]==1) {
							j4=true;
						}
					}
				}

				j3=false;

				i6=i7;

				while ((j3==false) && (i6<N4-1)) {
					if ((cool4[i6]==j4) && (Indif4[i6]==i8)) {
						j3=true;

						i10=M*i6;

						for (i3=0; (i3<i1) && (j3==true); i3++)
							if (Test[Kombi4[i3]]!=Vote4[i10+Kombi4[i3]]) {
								j3=false;
							}

						if (j3==true) {
							Value4[i6]=Value4[i6]+(NewValue*Value5[i9])/(totValue+0.0);
						}
					}

					i6=Test3[i6];
				}

				if (j3==false) {
					Value4[i4]=(NewValue*Value5[i9])/(totValue+0.0);

					i10=M*i4;

					for (i3=0; i3<M; i3++) {
						Vote4[i10+i3]=Test[i3];
					}

					Indif4[i4]=i8;
					cool4 [i4]=j4;

					i4++;
				}
			}

			N4=i4;
		}
	}
}

/*******************************************************************************/

void EKarp() {

	/* The basic idea to calculate the strength of a vote management is:          */
	/*                                                                            */
	/* 1. We calculate Vote7 and Value7. "Vote7[M*i+j]=true" if and only if       */
	/*    voter i strictly prefers candidate j to that candidate against whom     */
	/*    the vote management strategy is run.                                    */
	/*                                                                            */
	/* 2. We calculate Vote8 and Value8. "Vote8[M*i+j]=true" if and only if       */
	/*                                                                            */
	/*    (1) voter i strictly prefers candidate j to that candidate              */
	/*        against whom the vote management strategy is run or                 */
	/*                                                                            */
	/*    (2) he is indifferent between these two candidates.                     */
	/*                                                                            */
	/* 3. We solve the equation at stage 4 of section 5.3 of the paper "Free      */
	/*    Riding and Vote Management under Proportional Representation by the     */
	/*    Single Transferable Vote" both for Vote7 and Vote8. If the result is    */
	/*    identical for Vote7 and Vote8, then this is the strength of the vote    */
	/*    management and there is no need for further stages of proportional      */
	/*    completion.                                                             */
	/*                                                                            */
	/* For Voter7, we solve this equation as follows:                             */
	/*                                                                            */
	/* 1. In the beginning, UpperBounce7 = N/M and LowerBounce7 = 0.              */
	/*                                                                            */
	/* 2. We create the following digraph:                                        */
	/*                                                                            */
	/*    a) There is one vertex for each voter, one vertex for each candidate,   */
	/*       one vertex for the source, and one vertex for the drain.             */
	/*                                                                            */
	/*    b) From the source to voter i, there is an arc with capacity Value7[i]  */
	/*       (i.e. Arc7[k].start=source; Arc7[k].end=voter_i;                     */
	/*       Arc7[k].Cap=Value7[i]).                                              */
	/*                                                                            */
	/*    c) From voter i to candidate j, there is an arc with capacity Value7[i] */
	/*       if and only if "Vote7[M*i+j]=true" (i.e. Arc7[k].start=voter_i;      */
	/*       Arc7[k].end=candidate_j; Arc7[k].Cap=Value7[i]).                     */
	/*                                                                            */
	/*    d) From candidate j to the drain, there is an arc with capacity         */
	/*       UpperBounce7 (i.e. Arc7[k].start=candidate_j; Arc7[k].end=drain;     */
	/*       Arc7[k].Cap=UpperBounce7).                                           */
	/*                                                                            */
	/* 3. When we have calculated the maximum possible flow from the source       */
	/*    to the drain in the above digraph, we know that the solution of the     */
	/*    above equation must be between                                          */
	/*                                                                            */
	/*    X = the minimum flow over an arc from a candidate to the drain and      */
	/*                                                                            */
	/*    Y = ( the total flow to the drain ) / M.                                */
	/*                                                                            */
	/*    Therefore, LowerBounce7 is replaced by X and UpperBounce7 is            */
	/*    replaced by Y.                                                          */
	/*                                                                            */
	/* 4. We repeat stage 2 and 3, until the difference between LowerBounce7      */
	/*    and UpperBounce7 is below eps3.                                         */
	/*                                                                            */
	/* In the same manner, we solve this equation for Voter8.                     */
	/* If "LowerBounce7+5*eps3<UpperBounce8", then this is the strength of the    */
	/* vote management and there is no need for further stages of proportional    */
	/* completion.                                                                */


	/* Whenever the flow Arc[k].Value over arc k is between Arc[k].Cap-eps1 and   */
	/* Arc[k].Cap, it is set to Arc[k].Cap. Whenever the flow Arc[k].Value over   */
	/* arc k is between 0 and eps1, it is set to 0. This is necessary to prevent  */
	/* the values from circling caused by the finite precision of the entris.     */

	int i1,i2,i3,i4,i47,i48,i5,i57,i58,i6,i7,i8,i9;
	int NArcs7,NArcs8,NVertices7,NVertices8,Source7,Source8,Drain7,Drain8,
		first,
		gross;
	bool j1,j2,j3,j4,j5;
	long double Flow,PossFlow,d1,d2,d3;
	long double LowerBounce7,UpperBounce7,UpperBounce7a,
		 LowerBounce8,UpperBounce8,UpperBounce8a;

	tobecompleted=0;
	for (i1=1; i1<M; i1++) {
		if ((completed[i1]==false) && (completed[tobecompleted]==true)) {
			tobecompleted=i1;
		}
	}

	if (N4>Length7) {
		free(Vote7);
		free(Value7);
		free(Vote8);
		free(Value8);

		Length7=N4;

		Vote7 =(bool       *)calloc(Length7*M+1,sizeof(bool));
		Value7=(long double*)calloc(Length7+1,sizeof(long double));
		Vote8 =(bool       *)calloc(Length7*M+1,sizeof(bool));
		Value8=(long double*)calloc(Length7+1,sizeof(long double));
	}

	for (i1=0; i1<M; i1++) {
		if (Vote4[i1]==3) {
			Vote8[i1]=false;
		} else {
			Vote8[i1]=true;
		}
	}

	Value8[0]=Value4[0];

	N8=1;

	for (i1=1; i1<N4; i1++) {
		i2=0;
		j1=false;

		while ((i2<N8) && (j1==false)) {
			j1=true;

			for (i3=0; (i3<M) && (j1==true); i3++)
				if (((Vote8[M*i2+i3]==true) && (Vote4[M*i1+i3]==3))
					|| ((Vote8[M*i2+i3]==false) && (Vote4[M*i1+i3]!=3))) {
					j1=false;
				}

			if (j1==true) {
				Value8[i2]=Value8[i2]+Value4[i1];
			}

			i2++;
		}

		if (j1==false) {
			i4=M*N8;
			i5=M*i1;

			for (i3=0; i3<M; i3++) {
				if (Vote4[i5+i3]==3) {
					Vote8[i4+i3]=false;
				} else {
					Vote8[i4+i3]=true;
				}
			}

			Value8[N8]=Value4[i1];

			N8++;
		}
	}

	if (N8>LengthIndiff8) {
		free(Indiff8);
		LengthIndiff8=N8;
		Indiff8=(struct IndiffElement*)calloc(LengthIndiff8+1,
				sizeof(struct IndiffElement));
	}

	NArcs8=M;
	LowerBounce8 =0.0;
	UpperBounce8a=N+0.0;

	for (i1=0; i1<M; i1++) {
		Votes1[i1]=0;
	}

	i58=0;
	for (i1=0; i1<N8; i1++) {
		i2=1;

		for (i3=0; i3<M; i3++)
			if (Vote8[M*i1+i3]==true) {
				i2++;
				Votes1[i3]++;
			}

		Indiff8[i1].Value=i2;

		if (i2==1) {
			i48=i1;
			i58=1;
			UpperBounce8a=UpperBounce8a-Value8[i1];
		} else {
			NArcs8=NArcs8+i2;
		}
	}

	j3=true;
	for (i1=0; i1<M; i1++)
		if (Votes1[i1]==0) {
			j3=false;
		}

	if (j3==true) {
		for (i1=0; i1<M; i1++) {
			if (Vote4[i1]==1) {
				Vote7[i1]=true;
			} else {
				Vote7[i1]=false;
			}
		}

		Value7[0]=Value4[0];

		N7=1;

		for (i1=1; i1<N4; i1++) {
			i2=0;
			j1=false;

			while ((i2<N7) && (j1==false)) {
				j1=true;

				for (i3=0; (i3<M) && (j1==true); i3++)
					if (((Vote7[M*i2+i3]==true) && (Vote4[M*i1+i3]!=1))
						|| ((Vote7[M*i2+i3]==false) && (Vote4[M*i1+i3]==1))) {
						j1=false;
					}

				if (j1==true) {
					Value7[i2]=Value7[i2]+Value4[i1];
				}

				i2++;
			}

			if (j1==false) {
				i4=M*N7;
				i5=M*i1;

				for (i3=0; i3<M; i3++) {
					if (Vote4[i5+i3]==1) {
						Vote7[i4+i3]=true;
					} else {
						Vote7[i4+i3]=false;
					}
				}

				Value7[N7]=Value4[i1];

				N7++;
			}
		}

		NArcs7=M;
		LowerBounce7 =0.0;
		UpperBounce7a=N+0.0;

		if (N7>LengthIndiff7) {
			free(Indiff7);
			LengthIndiff7=N7;
			Indiff7=(struct IndiffElement*)calloc(LengthIndiff7+1,
					sizeof(struct IndiffElement));
		}

		for (i1=0; i1<M; i1++) {
			Votes1[i1]=0;
		}

		i57=0;
		for (i1=0; i1<N7; i1++) {
			i2=1;

			for (i3=0; i3<M; i3++)
				if (Vote7[M*i1+i3]==true) {
					i2++;
					Votes1[i3]++;
				}

			Indiff7[i1].Value=i2;

			if (i2==1) {
				i47=i1;
				i57=1;
				UpperBounce7a=UpperBounce7a-Value7[i1];
			} else {
				NArcs7=NArcs7+i2;
			}
		}

		j2=true;
		for (i1=0; i1<M; i1++)
			if (Votes1[i1]==0) {
				j2=false;

				if (completed[i1]==false) {
					tobecompleted=i1;
				}
			}

		if (NArcs7>LengthArc7) {
			free(Arcs7);
			LengthArc7=NArcs7;
			Arcs7=(struct ArcElement*)calloc(LengthArc7+1,sizeof(struct ArcElement));
		}

		if (NArcs8>LengthArc8) {
			free(Arcs8);
			LengthArc8=NArcs8;
			Arcs8=(struct ArcElement*)calloc(LengthArc8+1,sizeof(struct ArcElement));
		}

		NVertices7=N7-i57+2+M;
		if (NVertices7>LengthVertex7) {
			free(Vertices7);
			LengthVertex7=NVertices7;
			Vertices7=(struct VertexElement*)calloc(LengthVertex7+1,
					sizeof(struct VertexElement));
		}

		NVertices8=N8-i58+2+M;
		if (NVertices8>LengthVertex8) {
			free(Vertices8);
			LengthVertex8=NVertices8;
			Vertices8=(struct VertexElement*)calloc(LengthVertex8+1,
					sizeof(struct VertexElement));
		}

		Source7=N7-i57;
		Drain7 =N7+M+1-i57;
		Source8=N8-i58;
		Drain8 =N8+M+1-i58;

		UpperBounce7=UpperBounce7a/(M+0.0);
		UpperBounce8=UpperBounce8a/(M+0.0);

		if (j2==true) {
			Flow=0.0;

			for (i1=0; i1<M; i1++) {
				Votes[i1].Value=0.0;
			}

			for (i1=0; i1<N7; i1++) {
				Indiff7[i1].upper=i1-1;
				Indiff7[i1].lower=i1+1;
			}

			first=0;

			if (i57==1) {
				if (i47==0) {
					first=1;
					Indiff7[Indiff7[i47].lower].upper=Indiff7[i47].upper;
				} else {
					if (i47==N7-1) {
						Indiff7[Indiff7[i47].upper].lower=Indiff7[i47].lower;
					}

					else {
						Indiff7[Indiff7[i47].upper].lower=Indiff7[i47].lower;
						Indiff7[Indiff7[i47].lower].upper=Indiff7[i47].upper;
					}
				}
			}

			i9=0;

			for (i1=i57; i1<N7; i1++) {
				i2=first;
				i3=Indiff7[i2].lower;

				while (i3<N7) {
					if (Indiff7[i3].Value<Indiff7[i2].Value) {
						i2=i3;
					}

					i3=Indiff7[i3].lower;
				}

				if (i2==first) {
					first=Indiff7[i2].lower;
				}

				if (Indiff7[i2].lower!=N7) {
					Indiff7[Indiff7[i2].lower].upper=Indiff7[i2].upper;
				}

				if (Indiff7[i2].upper!=-1) {
					Indiff7[Indiff7[i2].upper].lower=Indiff7[i2].lower;
				}

				d1=Value7[i2];
				d3=0.0;

				for (i6=0; i6<M; i6++) {
					if (Vote7[M*i2+i6]==true) {
						Votes[i6].marked=false;
					} else {
						Votes[i6].marked=true;
					}
				}

				for (i7=1; i7<Indiff7[i2].Value; i7++) {
					i8=0;

					for (i6=0; i6<M; i6++)
						if (Votes[i6].marked==false)
							if ((Votes[i8].marked==true) || (Votes[i6].Value<=Votes[i8].Value)) {
								i8=i6;
							}

					Votes[i8].marked=true;

					d2=UpperBounce7-Votes[i8].Value;

					if (d2<0.0) {
						d2=0.0;
					}

					if (d2>d1) {
						d2=d1 ;
					}

					Arcs7[i9].Cap=Value7[i2];
					Arcs7[i9].Value =d2;

					if (Arcs7[i9].Cap-Arcs7[i9].Value<eps1) {
						Arcs7[i9].Value=Arcs7[i9].Cap;
					}

					if (Arcs7[i9].Value<eps1) {
						Arcs7[i9].Value=0.0;
					}

					d1=d1-d2;

					if (d1<eps1) {
						d1=0.0;
					}

					d3=d3+d2;

					if (Value7[i2]-d3<eps1) {
						d3=Value7[i2];
					}

					Votes[i8].Value=Votes[i8].Value+d2;

					if (UpperBounce7-Votes[i8].Value<eps1) {
						Votes[i8].Value=UpperBounce7;
					}

					Flow=Flow+d2;

					if (i57==1) {
						if (i2<i47) {
							Arcs7[i9].start=i2;
						} else {
							Arcs7[i9].start=i2-1;
						}
					} else {
						Arcs7[i9].start=i2;
					}

					Arcs7[i9].end=N7+1-i57+i8;

					i9++;
				}

				if (Indiff7[i2].Value>1) {
					Arcs7[i9].start =Source7;
					Arcs7[i9].Value =d3;
					Arcs7[i9].Cap=Value7[i2];

					if (i57==1) {
						if (i2<i47) {
							Arcs7[i9].end=i2;
						} else {
							Arcs7[i9].end=i2-1;
						}
					} else {
						Arcs7[i9].end=i2;
					}

					if (Arcs7[i9].Cap-Arcs7[i9].Value<eps1) {
						Arcs7[i9].Value=Arcs7[i9].Cap;
					}

					if (Arcs7[i9].Value<eps1) {
						Arcs7[i9].Value=0.0;
					}

					i9++;
				}
			}

			for (i1=0; i1<M; i1++) {
				Arcs7[i9].start =N7+1-i57+i1;
				Arcs7[i9].end   =Drain7;
				Arcs7[i9].Value =Votes[i1].Value;
				Arcs7[i9].Cap=UpperBounce7;

				if (Arcs7[i9].Cap-Arcs7[i9].Value<eps1) {
					Arcs7[i9].Value=Arcs7[i9].Cap;
				}

				if (Arcs7[i9].Value<eps1) {
					Arcs7[i9].Value=0.0;
				}

				i9++;
			}

			j1=true;
			gross=NArcs7+1;

			if (Flow<=UpperBounce7a-eps2)
				while (j1==true) {
					for (i1=0; i1<NVertices7; i1++) {
						Vertices7[i1].marked=gross;
					}

					Vertices7[Source7].marked=0;

					for (i1=0; i1<NArcs7; i1++) {
						Arcs7[i1].upper=i1-1;
						Arcs7[i1].lower=i1+1;
					}

					Arcs7[0].upper=NArcs7-1;
					Arcs7[NArcs7-1].lower=0;

					i1=NArcs7-1;

					j4=true;

					i5=0;
					j5=true;

					while ((Vertices7[Drain7].marked==gross) && (j5==true)) {
						i6=Arcs7[i1].start;
						i7=Arcs7[i1].end;

						if (Vertices7[i6].marked==i5) {
							if ((Vertices7[i7].marked==gross)
								&& (Arcs7[i1].Value<Arcs7[i1].Cap)) {
								Vertices7[i7].marked   =i5+1;
								Vertices7[i7].upper    =i1;
								Vertices7[i7].direction=true;

								j4=false;
							}

							Arcs7[Arcs7[i1].upper].lower=Arcs7[i1].lower;
							Arcs7[Arcs7[i1].lower].upper=Arcs7[i1].upper;
						}

						else {
							if (Vertices7[i7].marked==i5) {
								if ((Vertices7[i6].marked==gross) && (Arcs7[i1].Value>0.0)) {
									Vertices7[i6].marked   =i5+1;
									Vertices7[i6].upper    =i1;
									Vertices7[i6].direction=false;

									j4=false;
								}

								Arcs7[Arcs7[i1].upper].lower=Arcs7[i1].lower;
								Arcs7[Arcs7[i1].lower].upper=Arcs7[i1].upper;
							}
						}

						if (i1<Arcs7[i1].upper) {
							i5++;

							if (j4==true) {
								j5=false;
							}

							j4=true;
						}

						i1=Arcs7[i1].upper;
					}

					if (Vertices7[Drain7].marked<gross) {
						i1=Drain7;

						PossFlow=N+0.0;

						while (i1!=Source7) {
							i2=Vertices7[i1].upper;

							if (Vertices7[i1].direction==true) {
								if (Arcs7[i2].Cap-Arcs7[i2].Value<PossFlow)

								{
									PossFlow=Arcs7[i2].Cap-Arcs7[i2].Value;
								}

								i1=Arcs7[i2].start;
							}

							else {
								if (Arcs7[i2].Value<PossFlow)

								{
									PossFlow=Arcs7[i2].Value;
								}

								i1=Arcs7[i2].end;
							}
						}

						i1=Drain7;

						while (i1!=Source7) {
							i2=Vertices7[i1].upper;

							if (Vertices7[i1].direction==true) {
								Arcs7[i2].Value=Arcs7[i2].Value+PossFlow;

								if (Arcs7[i2].Value>Arcs7[i2].Cap-eps1)

								{
									Arcs7[i2].Value=Arcs7[i2].Cap;
								}

								i1=Arcs7[i2].start;
							}

							else {
								Arcs7[i2].Value=Arcs7[i2].Value-PossFlow;

								if (Arcs7[i2].Value<eps1)

								{
									Arcs7[i2].Value=0.0;
								}

								i1=Arcs7[i2].end;
							}
						}

						Flow=Flow+PossFlow;
					}

					else {
						j1=false;
					}

					if (Flow>UpperBounce7a-eps2) {
						j1=false;
					}
				}

			LowerBounce7=N+0.0;

			for (i1=0; i1<M; i1++) {
				if (Arcs7[NArcs7-M+i1].Value<LowerBounce7) {
					LowerBounce7=Arcs7[NArcs7-M+i1].Value;
				}

				if ((completed[i1]==false)
					&& (Arcs7[NArcs7-M+i1].Value<Arcs7[NArcs7-M+tobecompleted].Value)) {
					tobecompleted=i1;
				}
			}

			UpperBounce7a=Flow;
			UpperBounce7 =UpperBounce7a/(M+0.0);
		} else {
			LowerBounce7 =0.0;
			UpperBounce7 =0.0;
			UpperBounce7a=0.0;
		}

		Flow=0.0;

		for (i1=0; i1<M; i1++) {
			Votes[i1].Value=0.0;
		}

		for (i1=0; i1<N8; i1++) {
			Indiff8[i1].upper=i1-1;
			Indiff8[i1].lower=i1+1;
		}

		first=0;

		if (i58==1) {
			if (i48==0) {
				first=1;
				Indiff8[Indiff8[i48].lower].upper=Indiff8[i48].upper;
			} else {
				if (i48==N8-1) {
					Indiff8[Indiff8[i48].upper].lower=Indiff8[i48].lower;
				}

				else {
					Indiff8[Indiff8[i48].upper].lower=Indiff8[i48].lower;
					Indiff8[Indiff8[i48].lower].upper=Indiff8[i48].upper;
				}
			}
		}

		i9=0;

		for (i1=i58; i1<N8; i1++) {
			i2=first;
			i3=Indiff8[i2].lower;

			while (i3<N8) {
				if (Indiff8[i3].Value<Indiff8[i2].Value) {
					i2=i3;
				}

				i3=Indiff8[i3].lower;
			}

			if (i2==first) {
				first=Indiff8[i2].lower;
			}

			if (Indiff8[i2].lower!=N8) {
				Indiff8[Indiff8[i2].lower].upper=Indiff8[i2].upper;
			}

			if (Indiff8[i2].upper!=-1) {
				Indiff8[Indiff8[i2].upper].lower=Indiff8[i2].lower;
			}

			d1=Value8[i2];
			d3=0.0;

			for (i6=0; i6<M; i6++) {
				if (Vote8[M*i2+i6]==true) {
					Votes[i6].marked=false;
				} else {
					Votes[i6].marked=true;
				}
			}

			for (i7=1; i7<Indiff8[i2].Value; i7++) {
				i8=0;

				for (i6=0; i6<M; i6++)
					if (Votes[i6].marked==false)
						if ((Votes[i8].marked==true) || (Votes[i6].Value<=Votes[i8].Value)) {
							i8=i6;
						}

				Votes[i8].marked=true;

				d2=UpperBounce8-Votes[i8].Value;

				if (d2<0.0) {
					d2=0.0;
				}

				if (d2>d1) {
					d2=d1 ;
				}

				Arcs8[i9].Cap=Value8[i2];
				Arcs8[i9].Value =d2;

				if (Arcs8[i9].Cap-Arcs8[i9].Value<eps1) {
					Arcs8[i9].Value=Arcs8[i9].Cap;
				}

				if (Arcs8[i9].Value<eps1) {
					Arcs8[i9].Value=0.0;
				}

				d1=d1-d2;

				if (d1<eps1) {
					d1=0.0;
				}

				d3=d3+d2;

				if (Value8[i2]-d3<eps1) {
					d3=Value8[i2];
				}

				Votes[i8].Value=Votes[i8].Value+d2;

				if (UpperBounce8-Votes[i8].Value<eps1) {
					Votes[i8].Value=UpperBounce8;
				}

				Flow=Flow+d2;

				if (i58==1) {
					if (i2<i48) {
						Arcs8[i9].start=i2;
					} else {
						Arcs8[i9].start=i2-1;
					}
				} else {
					Arcs8[i9].start=i2;
				}

				Arcs8[i9].end=N8+1-i58+i8;

				i9++;
			}

			if (Indiff8[i2].Value>1) {
				Arcs8[i9].start =Source8;
				Arcs8[i9].Value =d3;
				Arcs8[i9].Cap=Value8[i2];

				if (i58==1) {
					if (i2<i48) {
						Arcs8[i9].end=i2;
					} else {
						Arcs8[i9].end=i2-1;
					}
				} else {
					Arcs8[i9].end=i2;
				}

				if (Arcs8[i9].Cap-Arcs8[i9].Value<eps1) {
					Arcs8[i9].Value=Arcs8[i9].Cap;
				}

				if (Arcs8[i9].Value<eps1) {
					Arcs8[i9].Value=0.0;
				}

				i9++;
			}
		}

		for (i1=0; i1<M; i1++) {
			Arcs8[i9].start =N8+1-i58+i1;
			Arcs8[i9].end   =Drain8;
			Arcs8[i9].Value =Votes[i1].Value;
			Arcs8[i9].Cap   =UpperBounce8;

			if (Arcs8[i9].Cap-Arcs8[i9].Value<eps1) {
				Arcs8[i9].Value=Arcs8[i9].Cap;
			}

			if (Arcs8[i9].Value<eps1) {
				Arcs8[i9].Value=0.0;
			}

			i9++;
		}

		j1=true;
		gross=NArcs8+1;

		if (Flow<=UpperBounce8a-eps2)
			while (j1==true) {
				for (i1=0; i1<NVertices8; i1++) {
					Vertices8[i1].marked=gross;
				}

				Vertices8[Source8].marked=0;

				for (i1=0; i1<NArcs8; i1++) {
					Arcs8[i1].upper=i1-1;
					Arcs8[i1].lower=i1+1;
				}

				Arcs8[0].upper=NArcs8-1;
				Arcs8[NArcs8-1].lower=0;

				i1=NArcs8-1;

				j4=true;

				i5=0;
				j5=true;

				while ((Vertices8[Drain8].marked==gross) && (j5==true)) {
					i6=Arcs8[i1].start;
					i7=Arcs8[i1].end;

					if (Vertices8[i6].marked==i5) {
						if ((Vertices8[i7].marked==gross)
							&& (Arcs8[i1].Value<Arcs8[i1].Cap)) {
							Vertices8[i7].marked   =i5+1;
							Vertices8[i7].upper    =i1;
							Vertices8[i7].direction=true;

							j4=false;
						}

						Arcs8[Arcs8[i1].upper].lower=Arcs8[i1].lower;
						Arcs8[Arcs8[i1].lower].upper=Arcs8[i1].upper;
					}

					else {
						if (Vertices8[i7].marked==i5) {
							if ((Vertices8[i6].marked==gross) && (Arcs8[i1].Value>0.0)) {
								Vertices8[i6].marked   =i5+1;
								Vertices8[i6].upper    =i1;
								Vertices8[i6].direction=false;

								j4=false;
							}

							Arcs8[Arcs8[i1].upper].lower=Arcs8[i1].lower;
							Arcs8[Arcs8[i1].lower].upper=Arcs8[i1].upper;
						}
					}

					if (i1<Arcs8[i1].upper) {
						i5++;

						if (j4==true) {
							j5=false;
						}

						j4=true;
					}

					i1=Arcs8[i1].upper;
				}

				if (Vertices8[Drain8].marked<gross) {
					i1=Drain8;

					PossFlow=N+0.0;

					while (i1!=Source8) {
						i2=Vertices8[i1].upper;

						if (Vertices8[i1].direction==true) {
							if (Arcs8[i2].Cap-Arcs8[i2].Value<PossFlow)

							{
								PossFlow=Arcs8[i2].Cap-Arcs8[i2].Value;
							}

							i1=Arcs8[i2].start;
						}

						else {
							if (Arcs8[i2].Value<PossFlow)

							{
								PossFlow=Arcs8[i2].Value;
							}

							i1=Arcs8[i2].end;
						}
					}

					i1=Drain8;

					while (i1!=Source8) {
						i2=Vertices8[i1].upper;

						if (Vertices8[i1].direction==true) {
							Arcs8[i2].Value=Arcs8[i2].Value+PossFlow;

							if (Arcs8[i2].Value>Arcs8[i2].Cap-eps1)

							{
								Arcs8[i2].Value=Arcs8[i2].Cap;
							}

							i1=Arcs8[i2].start;
						}

						else {
							Arcs8[i2].Value=Arcs8[i2].Value-PossFlow;

							if (Arcs8[i2].Value<eps1)

							{
								Arcs8[i2].Value=0.0;
							}

							i1=Arcs8[i2].end;
						}
					}

					Flow=Flow+PossFlow;
				}

				else {
					j1=false;
				}

				if (Flow>UpperBounce8a-eps2) {
					j1=false;
				}
			}

		LowerBounce8=N+0.0;
		for (i1=0; i1<M; i1++)
			if (Arcs8[NArcs8-M+i1].Value<LowerBounce8) {
				LowerBounce8=Arcs8[NArcs8-M+i1].Value;
			}

		UpperBounce8a=Flow;
		UpperBounce8 =UpperBounce8a/(M+0.0);

		while (((LowerBounce7+eps3<UpperBounce7)
				|| (LowerBounce8+eps3<UpperBounce8))
			&& (LowerBounce7+5.0*eps3<UpperBounce8)) {
			if (LowerBounce7+eps3<UpperBounce7) {
				Flow=0.0;

				for (i1=0; i1<M; i1++) {
					Votes[i1].Value=0.0;
				}

				for (i1=0; i1<N7; i1++) {
					Indiff7[i1].upper=i1-1;
					Indiff7[i1].lower=i1+1;
				}

				first=0;

				if (i57==1) {
					if (i47==0) {
						first=1;
						Indiff7[Indiff7[i47].lower].upper=Indiff7[i47].upper;
					} else {
						if (i47==N7-1) {
							Indiff7[Indiff7[i47].upper].lower=Indiff7[i47].lower;
						}

						else {
							Indiff7[Indiff7[i47].upper].lower=Indiff7[i47].lower;
							Indiff7[Indiff7[i47].lower].upper=Indiff7[i47].upper;
						}
					}
				}

				i9=0;

				for (i1=i57; i1<N7; i1++) {
					i2=first;
					i3=Indiff7[i2].lower;

					while (i3<N7) {
						if (Indiff7[i3].Value<Indiff7[i2].Value) {
							i2=i3;
						}

						i3=Indiff7[i3].lower;
					}

					if (i2==first) {
						first=Indiff7[i2].lower;
					}

					if (Indiff7[i2].lower!=N7) {
						Indiff7[Indiff7[i2].lower].upper=Indiff7[i2].upper;
					}

					if (Indiff7[i2].upper!=-1) {
						Indiff7[Indiff7[i2].upper].lower=Indiff7[i2].lower;
					}

					d1=Value7[i2];
					d3=0.0;

					for (i6=0; i6<M; i6++) {
						if (Vote7[M*i2+i6]==true) {
							Votes[i6].marked=false;
						} else {
							Votes[i6].marked=true;
						}
					}

					for (i7=1; i7<Indiff7[i2].Value; i7++) {
						i8=0;

						for (i6=0; i6<M; i6++)
							if (Votes[i6].marked==false)
								if ((Votes[i8].marked==true)
									|| (Votes[i6].Value<=Votes[i8].Value)) {
									i8=i6;
								}

						Votes[i8].marked=true;

						d2=UpperBounce7-Votes[i8].Value;

						if (d2<0.0) {
							d2=0.0;
						}

						if (d2>d1) {
							d2=d1 ;
						}

						Arcs7[i9].Value =d2;

						if (Arcs7[i9].Cap-Arcs7[i9].Value<eps1) {
							Arcs7[i9].Value=Arcs7[i9].Cap;
						}

						if (Arcs7[i9].Value<eps1) {
							Arcs7[i9].Value=0.0;
						}

						d1=d1-d2;

						if (d1<eps1) {
							d1=0.0;
						}

						d3=d3+d2;

						if (Value7[i2]-d3<eps1) {
							d3=Value7[i2];
						}

						Votes[i8].Value=Votes[i8].Value+d2;

						if (UpperBounce7-Votes[i8].Value<eps1) {
							Votes[i8].Value=UpperBounce7;
						}

						Flow=Flow+d2;

						Arcs7[i9].end=N7+1-i57+i8;

						i9++;
					}

					if (Indiff7[i2].Value>1) {
						Arcs7[i9].Value =d3;

						if (Arcs7[i9].Cap-Arcs7[i9].Value<eps1) {
							Arcs7[i9].Value=Arcs7[i9].Cap;
						}

						if (Arcs7[i9].Value<eps1) {
							Arcs7[i9].Value=0.0;
						}

						i9++;
					}
				}

				for (i1=0; i1<M; i1++) {
					Arcs7[i9].Value =Votes[i1].Value;
					Arcs7[i9].Cap=UpperBounce7;

					if (Arcs7[i9].Cap-Arcs7[i9].Value<eps1) {
						Arcs7[i9].Value=Arcs7[i9].Cap;
					}

					if (Arcs7[i9].Value<eps1) {
						Arcs7[i9].Value=0.0;
					}

					i9++;
				}

				j1=true;
				gross=NArcs7+1;

				if (Flow<=UpperBounce7a-eps2)
					while (j1==true) {
						for (i1=0; i1<NVertices7; i1++) {
							Vertices7[i1].marked=gross;
						}

						Vertices7[Source7].marked=0;

						for (i1=0; i1<NArcs7; i1++) {
							Arcs7[i1].upper=i1-1;
							Arcs7[i1].lower=i1+1;
						}

						Arcs7[0].upper=NArcs7-1;
						Arcs7[NArcs7-1].lower=0;

						i1=NArcs7-1;

						j4=true;

						i5=0;
						j5=true;

						while ((Vertices7[Drain7].marked==gross) && (j5==true)) {
							i6=Arcs7[i1].start;
							i7=Arcs7[i1].end;

							if (Vertices7[i6].marked==i5) {
								if ((Vertices7[i7].marked==gross)
									&& (Arcs7[i1].Value<Arcs7[i1].Cap)) {
									Vertices7[i7].marked   =i5+1;
									Vertices7[i7].upper    =i1;
									Vertices7[i7].direction=true;

									j4=false;
								}

								Arcs7[Arcs7[i1].upper].lower=Arcs7[i1].lower;
								Arcs7[Arcs7[i1].lower].upper=Arcs7[i1].upper;
							}

							else {
								if (Vertices7[i7].marked==i5) {
									if ((Vertices7[i6].marked==gross)
										&& (Arcs7[i1].Value>0.0)) {
										Vertices7[i6].marked   =i5+1;
										Vertices7[i6].upper    =i1;
										Vertices7[i6].direction=false;

										j4=false;
									}

									Arcs7[Arcs7[i1].upper].lower=Arcs7[i1].lower;
									Arcs7[Arcs7[i1].lower].upper=Arcs7[i1].upper;
								}
							}

							if (i1<Arcs7[i1].upper) {
								i5++;

								if (j4==true) {
									j5=false;
								}

								j4=true;
							}

							i1=Arcs7[i1].upper;
						}

						if (Vertices7[Drain7].marked<gross) {
							i1=Drain7;

							PossFlow=N+0.0;

							while (i1!=Source7) {
								i2=Vertices7[i1].upper;

								if (Vertices7[i1].direction==true) {
									if (Arcs7[i2].Cap-Arcs7[i2].Value<PossFlow)

									{
										PossFlow=Arcs7[i2].Cap-Arcs7[i2].Value;
									}

									i1=Arcs7[i2].start;
								}

								else {
									if (Arcs7[i2].Value<PossFlow)

									{
										PossFlow=Arcs7[i2].Value;
									}

									i1=Arcs7[i2].end;
								}
							}

							i1=Drain7;

							while (i1!=Source7) {
								i2=Vertices7[i1].upper;

								if (Vertices7[i1].direction==true) {
									Arcs7[i2].Value=Arcs7[i2].Value+PossFlow;

									if (Arcs7[i2].Value>Arcs7[i2].Cap-eps1)

									{
										Arcs7[i2].Value=Arcs7[i2].Cap;
									}

									i1=Arcs7[i2].start;
								}

								else {
									Arcs7[i2].Value=Arcs7[i2].Value-PossFlow;

									if (Arcs7[i2].Value<eps1)

									{
										Arcs7[i2].Value=0.0;
									}

									i1=Arcs7[i2].end;
								}
							}

							Flow=Flow+PossFlow;
						}

						else {
							j1=false;
						}

						if (Flow>UpperBounce7a-eps2) {
							j1=false;
						}
					}

				LowerBounce7=N+0.0;

				for (i1=0; i1<M; i1++) {
					if (Arcs7[NArcs7-M+i1].Value<LowerBounce7) {
						LowerBounce7=Arcs7[NArcs7-M+i1].Value;
					}

					if ((completed[i1]==false)
						&& (Arcs7[NArcs7-M+i1].Value<Arcs7[NArcs7-M+tobecompleted].Value)) {
						tobecompleted=i1;
					}
				}

				UpperBounce7a=Flow;
				UpperBounce7 =UpperBounce7a/(M+0.0);
			}

			if (LowerBounce8+eps3<UpperBounce8) {
				Flow=0.0;

				for (i1=0; i1<M; i1++) {
					Votes[i1].Value=0.0;
				}

				for (i1=0; i1<N8; i1++) {
					Indiff8[i1].upper=i1-1;
					Indiff8[i1].lower=i1+1;
				}

				first=0;

				if (i58==1) {
					if (i48==0) {
						first=1;
						Indiff8[Indiff8[i48].lower].upper=Indiff8[i48].upper;
					} else {
						if (i48==N8-1) {
							Indiff8[Indiff8[i48].upper].lower=Indiff8[i48].lower;
						}

						else {
							Indiff8[Indiff8[i48].upper].lower=Indiff8[i48].lower;
							Indiff8[Indiff8[i48].lower].upper=Indiff8[i48].upper;
						}
					}
				}

				i9=0;

				for (i1=i58; i1<N8; i1++) {
					i2=first;
					i3=Indiff8[i2].lower;

					while (i3<N8) {
						if (Indiff8[i3].Value<Indiff8[i2].Value) {
							i2=i3;
						}

						i3=Indiff8[i3].lower;
					}

					if (i2==first) {
						first=Indiff8[i2].lower;
					}

					if (Indiff8[i2].lower!=N8) {
						Indiff8[Indiff8[i2].lower].upper=Indiff8[i2].upper;
					}

					if (Indiff8[i2].upper!=-1) {
						Indiff8[Indiff8[i2].upper].lower=Indiff8[i2].lower;
					}

					d1=Value8[i2];
					d3=0.0;

					for (i6=0; i6<M; i6++) {
						if (Vote8[M*i2+i6]==true) {
							Votes[i6].marked=false;
						} else {
							Votes[i6].marked=true;
						}
					}

					for (i7=1; i7<Indiff8[i2].Value; i7++) {
						i8=0;

						for (i6=0; i6<M; i6++)
							if (Votes[i6].marked==false)
								if ((Votes[i8].marked==true)
									|| (Votes[i6].Value<=Votes[i8].Value)) {
									i8=i6;
								}

						Votes[i8].marked=true;

						d2=UpperBounce8-Votes[i8].Value;

						if (d2<0.0) {
							d2=0.0;
						}

						if (d2>d1) {
							d2=d1 ;
						}

						Arcs8[i9].Value =d2;

						if (Arcs8[i9].Cap-Arcs8[i9].Value<eps1) {
							Arcs8[i9].Value=Arcs8[i9].Cap;
						}

						if (Arcs8[i9].Value<eps1) {
							Arcs8[i9].Value=0.0;
						}

						d1=d1-d2;

						if (d1<eps1) {
							d1=0.0;
						}

						d3=d3+d2;

						if (Value8[i2]-d3<eps1) {
							d3=Value8[i2];
						}

						Votes[i8].Value=Votes[i8].Value+d2;

						if (UpperBounce8-Votes[i8].Value<eps1) {
							Votes[i8].Value=UpperBounce8;
						}

						Flow=Flow+d2;

						Arcs8[i9].end=N8+1-i58+i8;

						i9++;
					}

					if (Indiff8[i2].Value>1) {
						Arcs8[i9].Value =d3;

						if (Arcs8[i9].Cap-Arcs8[i9].Value<eps1) {
							Arcs8[i9].Value=Arcs8[i9].Cap;
						}

						if (Arcs8[i9].Value<eps1) {
							Arcs8[i9].Value=0.0;
						}

						i9++;
					}
				}

				for (i1=0; i1<M; i1++) {
					Arcs8[i9].Value =Votes[i1].Value;
					Arcs8[i9].Cap=UpperBounce8;

					if (Arcs8[i9].Cap-Arcs8[i9].Value<eps1) {
						Arcs8[i9].Value=Arcs8[i9].Cap;
					}

					if (Arcs8[i9].Value<eps1) {
						Arcs8[i9].Value=0.0;
					}

					i9++;
				}

				j1=true;
				gross=NArcs8+1;

				if (Flow<=UpperBounce8a-eps2)
					while (j1==true) {
						for (i1=0; i1<NVertices8; i1++) {
							Vertices8[i1].marked=gross;
						}

						Vertices8[Source8].marked=0;

						for (i1=0; i1<NArcs8; i1++) {
							Arcs8[i1].upper=i1-1;
							Arcs8[i1].lower=i1+1;
						}

						Arcs8[0].upper=NArcs8-1;
						Arcs8[NArcs8-1].lower=0;

						i1=NArcs8-1;

						j4=true;

						i5=0;
						j5=true;

						while ((Vertices8[Drain8].marked==gross) && (j5==true)) {
							i6=Arcs8[i1].start;
							i7=Arcs8[i1].end;

							if (Vertices8[i6].marked==i5) {
								if ((Vertices8[i7].marked==gross)
									&& (Arcs8[i1].Value<Arcs8[i1].Cap)) {
									Vertices8[i7].marked   =i5+1;
									Vertices8[i7].upper    =i1;
									Vertices8[i7].direction=true;

									j4=false;
								}

								Arcs8[Arcs8[i1].upper].lower=Arcs8[i1].lower;
								Arcs8[Arcs8[i1].lower].upper=Arcs8[i1].upper;
							}

							else {
								if (Vertices8[i7].marked==i5) {
									if ((Vertices8[i6].marked==gross)
										&& (Arcs8[i1].Value>0.0)) {
										Vertices8[i6].marked   =i5+1;
										Vertices8[i6].upper    =i1;
										Vertices8[i6].direction=false;

										j4=false;
									}

									Arcs8[Arcs8[i1].upper].lower=Arcs8[i1].lower;
									Arcs8[Arcs8[i1].lower].upper=Arcs8[i1].upper;
								}
							}

							if (i1<Arcs8[i1].upper) {
								i5++;

								if (j4==true) {
									j5=false;
								}

								j4=true;
							}

							i1=Arcs8[i1].upper;
						}

						if (Vertices8[Drain8].marked<gross) {
							i1=Drain8;

							PossFlow=N+0.0;

							while (i1!=Source8) {
								i2=Vertices8[i1].upper;

								if (Vertices8[i1].direction==true) {
									if (Arcs8[i2].Cap-Arcs8[i2].Value<PossFlow)

									{
										PossFlow=Arcs8[i2].Cap-Arcs8[i2].Value;
									}

									i1=Arcs8[i2].start;
								}

								else {
									if (Arcs8[i2].Value<PossFlow)

									{
										PossFlow=Arcs8[i2].Value;
									}

									i1=Arcs8[i2].end;
								}
							}

							i1=Drain8;

							while (i1!=Source8) {
								i2=Vertices8[i1].upper;

								if (Vertices8[i1].direction==true) {
									Arcs8[i2].Value=Arcs8[i2].Value+PossFlow;

									if (Arcs8[i2].Value>Arcs8[i2].Cap-eps1)

									{
										Arcs8[i2].Value=Arcs8[i2].Cap;
									}

									i1=Arcs8[i2].start;
								}

								else {
									Arcs8[i2].Value=Arcs8[i2].Value-PossFlow;

									if (Arcs8[i2].Value<eps1)

									{
										Arcs8[i2].Value=0.0;
									}

									i1=Arcs8[i2].end;
								}
							}

							Flow=Flow+PossFlow;
						}

						else {
							j1=false;
						}

						if (Flow>UpperBounce8a-eps2) {
							j1=false;
						}
					}

				LowerBounce8=N+0.0;
				for (i1=0; i1<M; i1++)
					if (Arcs8[NArcs8-M+i1].Value<LowerBounce8) {
						LowerBounce8=Arcs8[NArcs8-M+i1].Value;
					}

				UpperBounce8a=Flow;
				UpperBounce8 =UpperBounce8a/(M+0.0);
			}
		}

		if (LowerBounce7+5.0*eps3>=UpperBounce8) {
			SimplexNoetig=false;
			Output1=(LowerBounce7+UpperBounce8)/2.0;
			SimplexNichtNoetig++;
		}
	} else {
		SimplexNoetig=false;
		Output1=0.0;
		SimplexNichtNoetig++;
	}
}

/*******************************************************************************/

void EKarpSimple() {
	int i1,i2,i3,i4,i5,i6,i7,i8,i9;
	int NArcs,NVertices,Source,Drain,first,gross;
	bool j1,j2,j4,j5;
	long double Flow,PossFlow,MaxPossFlow,MaxPossFlow2,d1,d2,d3;

	NArcs=M;
	Flow=0.0;
	MaxPossFlow=N+0.0;
	i5=0;

	if (N4>Length6) {
		free(Indiff);
		Length6=N4;
		Indiff=(struct IndiffElement*)calloc(Length6+1,
				sizeof(struct IndiffElement));
	}

	for (i1=0; i1<M; i1++) {
		Votes1[i1]=0;
	}

	for (i1=0; i1<N4; i1++) {
		i2=1;

		for (i3=0; i3<M; i3++)
			if (Vote4[M*i1+i3]==1) {
				i2++;
				Votes1[i3]++;
			}

		Indiff[i1].Value=i2;

		if (i2==1) {
			i4=i1;
			i5=1;
			MaxPossFlow=MaxPossFlow-Value4[i1];
		} else {
			NArcs=NArcs+i2;
		}
	}

	j2=true;
	for (i1=0; i1<M; i1++)
		if (Votes1[i1]==0) {
			j2=false;
		}

	if (j2==true) {
		for (i1=0; i1<M; i1++) {
			Votes[i1].Value=0.0;
		}

		if (NArcs>LengthArc) {
			free(Arcs);
			LengthArc=NArcs;
			Arcs=(struct ArcElement*)calloc(LengthArc+1,sizeof(struct ArcElement));
		}

		NVertices=N4-i5+2+M;
		if (NVertices>LengthVertex) {
			free(Vertices);
			LengthVertex=NVertices;
			Vertices=(struct VertexElement*)calloc(LengthVertex+1,
					sizeof(struct VertexElement));
		}

		Source=N4-i5;
		Drain =N4+M+1-i5;
		MaxPossFlow2=MaxPossFlow/(M+0.0);

		for (i1=0; i1<N4; i1++) {
			Indiff[i1].upper=i1-1;
			Indiff[i1].lower=i1+1;
		}

		first=0;

		if (i5==1) {
			if (i4==0) {
				first=1;
				Indiff[Indiff[i4].lower].upper=Indiff[i4].upper;
			} else {
				if (i4==N4-1) {
					Indiff[Indiff[i4].upper].lower=Indiff[i4].lower;
				}

				else {
					Indiff[Indiff[i4].upper].lower=Indiff[i4].lower;
					Indiff[Indiff[i4].lower].upper=Indiff[i4].upper;
				}
			}
		}

		i9=0;

		for (i1=i5; i1<N4; i1++) {
			i2=first;
			i3=Indiff[i2].lower;

			while (i3<N4) {
				if (Indiff[i3].Value<Indiff[i2].Value) {
					i2=i3;
				}

				i3=Indiff[i3].lower;
			}

			if (i2==first) {
				first=Indiff[i2].lower;
			}

			if (Indiff[i2].lower!=N4) {
				Indiff[Indiff[i2].lower].upper=Indiff[i2].upper;
			}

			if (Indiff[i2].upper!=-1) {
				Indiff[Indiff[i2].upper].lower=Indiff[i2].lower;
			}

			d1=Value4[i2];
			d3=0.0;

			for (i6=0; i6<M; i6++) {
				if (Vote4[M*i2+i6]==1) {
					Votes[i6].marked=false;
				} else {
					Votes[i6].marked=true;
				}
			}

			for (i7=1; i7<Indiff[i2].Value; i7++) {
				i8=0;

				for (i6=0; i6<M; i6++)
					if (Votes[i6].marked==false)
						if ((Votes[i8].marked==true) || (Votes[i6].Value<=Votes[i8].Value)) {
							i8=i6;
						}

				Votes[i8].marked=true;

				d2=MaxPossFlow2-Votes[i8].Value;

				if (d2<0.0) {
					d2=0.0;
				}

				if (d2>d1) {
					d2=d1 ;
				}

				Arcs[i9].Cap=Value4[i2];
				Arcs[i9].Value =d2;

				if (Arcs[i9].Cap-Arcs[i9].Value<eps1) {
					Arcs[i9].Value=Arcs[i9].Cap;
				}

				if (Arcs[i9].Value<eps1) {
					Arcs[i9].Value=0.0;
				}

				d1=d1-d2;

				if (d1<eps1) {
					d1=0.0;
				}

				d3=d3+d2;

				if (Value4[i2]-d3<eps1) {
					d3=Value4[i2];
				}

				Votes[i8].Value=Votes[i8].Value+d2;

				if (MaxPossFlow2-Votes[i8].Value<eps1) {
					Votes[i8].Value=MaxPossFlow2;
				}

				Flow=Flow+d2;

				if (i5==1) {
					if (i2<i4) {
						Arcs[i9].start=i2;
					} else {
						Arcs[i9].start=i2-1;
					}
				} else {
					Arcs[i9].start=i2;
				}

				Arcs[i9].end=N4+1-i5+i8;

				i9++;
			}

			if (Indiff[i2].Value>1) {
				Arcs[i9].start =Source;
				Arcs[i9].Value =d3;
				Arcs[i9].Cap   =Value4[i2];

				if (i5==1) {
					if (i2<i4) {
						Arcs[i9].end=i2;
					} else {
						Arcs[i9].end=i2-1;
					}
				} else {
					Arcs[i9].end=i2;
				}

				if (Arcs[i9].Cap-Arcs[i9].Value<eps1) {
					Arcs[i9].Value=Arcs[i9].Cap;
				}

				if (Arcs[i9].Value<eps1) {
					Arcs[i9].Value=0.0;
				}

				i9++;
			}
		}

		for (i1=0; i1<M; i1++) {
			Arcs[i9].start =N4+1-i5+i1;
			Arcs[i9].end   =Drain;
			Arcs[i9].Value =Votes[i1].Value;
			Arcs[i9].Cap   =MaxPossFlow2;

			if (Arcs[i9].Cap-Arcs[i9].Value<eps1) {
				Arcs[i9].Value=Arcs[i9].Cap;
			}

			if (Arcs[i9].Value<eps1) {
				Arcs[i9].Value=0.0;
			}

			i9++;
		}

		j1=true;
		gross=NArcs+1;

		if (Flow<=MaxPossFlow-eps2)
			while (j1==true) {
				for (i1=0; i1<NVertices; i1++) {
					Vertices[i1].marked=gross;
				}

				Vertices[Source].marked=0;

				for (i1=0; i1<NArcs; i1++) {
					Arcs[i1].upper=i1-1;
					Arcs[i1].lower=i1+1;
				}

				Arcs[0].upper=NArcs-1;
				Arcs[NArcs-1].lower=0;

				i1=NArcs-1;

				j4=true;

				i5=0;
				j5=true;

				while ((Vertices[Drain].marked==gross) && (j5==true)) {
					i6=Arcs[i1].start;
					i7=Arcs[i1].end;

					if (Vertices[i6].marked==i5) {
						if ((Vertices[i7].marked==gross)
							&& (Arcs[i1].Value<Arcs[i1].Cap)) {
							Vertices[i7].marked   =i5+1;
							Vertices[i7].upper    =i1;
							Vertices[i7].direction=true;

							j4=false;
						}

						Arcs[Arcs[i1].upper].lower=Arcs[i1].lower;
						Arcs[Arcs[i1].lower].upper=Arcs[i1].upper;
					}

					else {
						if (Vertices[i7].marked==i5) {
							if ((Vertices[i6].marked==gross) && (Arcs[i1].Value>0.0)) {
								Vertices[i6].marked   =i5+1;
								Vertices[i6].upper    =i1;
								Vertices[i6].direction=false;

								j4=false;
							}

							Arcs[Arcs[i1].upper].lower=Arcs[i1].lower;
							Arcs[Arcs[i1].lower].upper=Arcs[i1].upper;
						}
					}

					if (i1<Arcs[i1].upper) {
						i5++;

						if (j4==true) {
							j5=false;
						}

						j4=true;
					}


					i1=Arcs[i1].upper;
				}

				if (Vertices[Drain].marked<gross) {
					i1=Drain;

					PossFlow=N+0.0;

					while (i1!=Source) {
						i2=Vertices[i1].upper;

						if (Vertices[i1].direction==true) {
							if (Arcs[i2].Cap-Arcs[i2].Value<PossFlow)

							{
								PossFlow=Arcs[i2].Cap-Arcs[i2].Value;
							}

							i1=Arcs[i2].start;
						}

						else {
							if (Arcs[i2].Value<PossFlow)

							{
								PossFlow=Arcs[i2].Value;
							}

							i1=Arcs[i2].end;
						}
					}

					i1=Drain;

					while (i1!=Source) {
						i2=Vertices[i1].upper;

						if (Vertices[i1].direction==true) {
							Arcs[i2].Value=Arcs[i2].Value+PossFlow;

							if (Arcs[i2].Value>Arcs[i2].Cap-eps1)

							{
								Arcs[i2].Value=Arcs[i2].Cap;
							}

							i1=Arcs[i2].start;
						}

						else {
							Arcs[i2].Value=Arcs[i2].Value-PossFlow;

							if (Arcs[i2].Value<eps1)

							{
								Arcs[i2].Value=0.0;
							}

							i1=Arcs[i2].end;
						}
					}

					Flow=Flow+PossFlow;
				}

				else {
					j1=false;
				}

				if (Flow>MaxPossFlow-eps2) {
					j1=false;
				}
			}

		if (Flow>MaxPossFlow-eps2) {
			SimplexNoetig=false;
			Output1=MaxPossFlow2;
			SimplexNichtNoetig++;
		} else {
			EKarp();
		}
	} else {
		EKarp();
	}
}

/*******************************************************************************/

void PropCompletion() {
	int i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11;
	bool j1,j2,j3,j4;
	long double NewValue;
	int totValue;

	j1=false;

	/* "j1=false" means that proportional completion hasn't yet been finished.    */

	while (j1==false) {
		/* Each Iteration of the proportional completion process consists of       */
		/* 3 stages.                                                               */

		/* At the first stage, we have to decide which voter should be completed   */
		/* next. We complete candidate i2 for whom Indif4[i2] is maximum.          */
		/* Indif4[i2] is the number of those candidates who get the same           */
		/* ranking from voter i2 as that candidate against whom this vote          */
		/* management strategy is run.                                             */

		j1=true;

		i1=0;
		i2=0;

		for (i3=0; i3<N4; i3++)
			if (Vote4[M*i3+tobecompleted]==2)
				if (Indif4[i3]>i1) {
					j1=false;
					i1=Indif4[i3];
					i2=i3;
				}

		/* Suppose (F(1),...,F(i1)) is that set of candidates such that voter i2   */
		/* is indifferent between each candidate in (F(1),...,F(i1)) and that      */
		/* candidate against whom this vote management strategy is run.            */

		/* Then, at the second stage, we check how those voters, who are not       */
		/* indifferent between all the candidates in (F(1),...,F(i1)) and that     */
		/* candidate against whom this vote management strategy is run, rank these */
		/* candidates. totValue is the number of voters who are not indifferent    */
		/* between these candidates.                                               */

		if (j1==false) {
			NewValue=Value4[i2];

			i10=M*i2;
			for (i3=0; i3<M; i3++) {
				Test[i3]=Vote4[i10+i3];
			}

			i4=0;
			for (i3=0; i3<M; i3++)
				if (Test[i3]==2) {
					Kombi4[i4]=i3;
					i4++;
				}

			totValue=0;
			i5=0;

			for (i3=0; i3<N3; i3++) {
				j2=false;

				for (i4=0; (i4<i1) && (j2==false); i4++)
					if (Vote3[M*i3+Kombi4[i4]]!=2) {
						j2=true;
					}

				if (j2==true) {
					totValue=totValue+Value3[i3];

					i6=0;
					j2=false;

					while ((j2==false) && (i6<i5)) {
						j2=true;

						for (i4=0; (i4<i1) && (j2==true); i4++)
							if (Vote3[M*i3+Kombi4[i4]]!=Vote5[i1*i6+i4]) {
								j2=false;
							}

						if (j2==true) {
							Value5[i6]=Value5[i6]+Value3[i3];
						}

						i6++;
					}

					if (j2==false) {
						for (i4=0; i4<i1; i4++) {
							Vote5[i1*i5+i4]=Vote3[M*i3+Kombi4[i4]];
						}

						Value5[i5]=Value3[i3];

						i5++;
					}
				}
			}

			if (totValue==0) {
				i5=i1+1;

				totValue=i1+1;

				for (i3=0; i3<i1; i3++) {
					for (i4=0; i4<i1; i4++) {
						Vote5[i1*i3+i4]=2;
					}

					Vote5[i1*i3+i3]=1;

					Value5[i3]=1;
				}

				for (i3=0; i3<i1; i3++) {
					Vote5[i1*i1+i3]=3;
				}

				Value5[i1]=1;
			}

			/* At the third stage, the original voter i2 is replaced by its         */
			/* proportionally completed correspondences.                            */

			i7=N4+i5;

			if (i7>Length4) {
				Vote6 =(unsigned char*)calloc(N4*M+1,sizeof(unsigned char));
				Value6=(long double  *)calloc(N4+1,sizeof(long double));
				Indif6=(int          *)calloc(N4+1,sizeof(int));

				for (i3=0; i3<N4; i3++) {
					Value6[i3]=Value4[i3];
					Indif6[i3]=Indif4[i3];

					i10=M*i3;
					for (i4=0; i4<M; i4++) {
						i11=i10+i4;
						Vote6[i11]=Vote4[i11];
					}
				}

				free(Vote4);
				free(Value4);
				free(Indif4);
				free(cool4);

				Length4=i7;

				Vote4 =(unsigned char*)calloc(Length4*M+1,sizeof(unsigned char));
				Value4=(long double  *)calloc(Length4+1,sizeof(long double));
				Indif4=(int          *)calloc(Length4+1,sizeof(int));
				cool4 =(bool         *)calloc(Length4+1,sizeof(bool));

				for (i3=0; i3<N4; i3++) {
					Value4[i3]=Value6[i3];
					Indif4[i3]=Indif6[i3];

					i10=M*i3;
					for (i4=0; i4<M; i4++) {
						i11=i10+i4;
						Vote4[i11]=Vote6[i11];
					}
				}

				free(Vote6);
				free(Value6);
				free(Indif6);

				free(Test3);
				Test3=(int*)calloc(Length4+1,sizeof(int));
			}

			i7=N4-1;
			Value4[i2]=Value4[i7];
			Indif4[i2]=Indif4[i7];

			i6=M*i2;
			i7=M*i7;
			for (i3=0; i3<M; i3++) {
				Vote4[i6+i3]=Vote4[i7+i3];
			}

			i7=N4-1;
			j3=false;

			for (i9=0; i9<N4-1; i9++) {
				j4=true;

				for (i3=0; (i3<M) && (j4==true); i3++)
					if (Test[i3]!=2)
						if (Test[i3]!=Vote4[M*i9+i3]) {
							j4=false;
						}

				if (j4==true) {
					if (j3==false) {
						j3=true;
						i7=i9;
						i6=i9;
					} else {
						Test3[i6]=i9;
						i6=i9;
					}
				}
			}

			if (j3==true) {
				Test3[i6]=N4-1;
			}

			i4=N4-1;

			for (i9=0; i9<i5; i9++) {
				for (i3=0; i3<i1; i3++) {
					Test[Kombi4[i3]]=Vote5[i1*i9+i3];
				}

				i8=0;
				j4=false;

				for (i3=0; i3<M; i3++)
					if (Test[i3]==2) {
						i8++;
					}

				j3=false;

				i6=i7;

				while ((j3==false) && (i6<N4-1)) {
					if (Indif4[i6]==i8) {
						j3=true;

						i10=M*i6;

						for (i3=0; (i3<i1) && (j3==true); i3++)
							if (Test[Kombi4[i3]]!=Vote4[i10+Kombi4[i3]]) {
								j3=false;
							}

						if (j3==true) {
							Value4[i6]=Value4[i6]+(NewValue*Value5[i9])/(totValue+0.0);
						}
					}

					i6=Test3[i6];
				}

				if (j3==false) {
					Value4[i4]=(NewValue*Value5[i9])/(totValue+0.0);

					i10=M*i4;

					for (i3=0; i3<M; i3++) {
						Vote4[i10+i3]=Test[i3];
					}

					Indif4[i4]=i8;

					i4++;
				}
			}

			N4=i4;
		}
	}
}

/*******************************************************************************/

void VoteMana() {
	int i1,i2,i3,i4,i5,i6,i7,i8;
	unsigned char i9;
	bool j1,j2;

	i1=Vote2[Kombi[g2]];
	i8=0;
	j2=false;

	for (i2=0; i2<g2; i2++) {
		if (Vote2[Kombi[i2]]<i1) {
			Vote3[i2]=1;
			Vote4[i2]=1;
			j2=true;
		} else {
			if (Vote2[Kombi[i2]]>i1) {
				Vote3[i2]=3;
				Vote4[i2]=3;
			} else {
				Vote3[i2]=2;
				Vote4[i2]=2;
				i8++;
			}
		}
	}

	for (i2=g2+1; i2<=M; i2++) {
		if (Vote2[Kombi[i2]]<i1) {
			Vote3[i2-1]=1;
			Vote4[i2-1]=1;
			j2=true;
		} else {
			if (Vote2[Kombi[i2]]>i1) {
				Vote3[i2-1]=3;
				Vote4[i2-1]=3;
			} else {
				Vote3[i2-1]=2;
				Vote4[i2-1]=2;
				i8++;
			}
		}
	}

	Value3[0]=Value2[0];
	Indif4[0]=i8;
	cool4 [0]=j2;

	N3=0;

	for (i1=1; i1<N2; i1++) {
		i2=0;

		i4=C*i1;

		j1=false;

		while ((i2<=N3) && (j1==false)) {
			i5=M*i2;

			j1=true;

			i6=Vote2[i4+Kombi[g2]];

			for (i3=0; (j1==true) && (i3<g2); i3++) {
				i7=Vote2[i4+Kombi[i3]];
				i9=Vote3[i5+i3];

				if (((i7>i6) && (i9!=3))
					|| ((i7<i6) && (i9!=1))
					|| ((i7==i6) && (i9!=2))) {
					j1=false;
				}
			}

			if (j1==true)
				for (i3=g2+1; (j1==true) && (i3<=M); i3++) {
					i7=Vote2[i4+Kombi[i3]];
					i9=Vote3[i5+i3-1];

					if (((i7>i6) && (i9!=3))
						|| ((i7<i6) && (i9!=1))
						|| ((i7==i6) && (i9!=2))) {
						j1=false;
					}
				}

			if (j1==false) {
				i2++;
			} else {
				Value3[i2]=Value3[i2]+Value2[i1];
			}
		}

		if (j1==false) {
			N3++;

			i5=M*N3;
			i6=Vote2[i4+Kombi[g2]];

			i8=0;
			j2=false;

			for (i2=0; i2<g2; i2++) {
				if (Vote2[i4+Kombi[i2]]<i6) {
					Vote3[i5+i2]=1;
					Vote4[i5+i2]=1;
					j2=true;
				} else {
					if (Vote2[i4+Kombi[i2]]>i6) {
						Vote3[i5+i2]=3;
						Vote4[i5+i2]=3;
					} else {
						Vote3[i5+i2]=2;
						Vote4[i5+i2]=2;
						i8++;
					}
				}
			}

			for (i2=g2+1; i2<=M; i2++) {
				if (Vote2[i4+Kombi[i2]]<i6) {
					Vote3[i5+i2-1]=1;
					Vote4[i5+i2-1]=1;
					j2=true;
				} else {
					if (Vote2[i4+Kombi[i2]]>i6) {
						Vote3[i5+i2-1]=3;
						Vote4[i5+i2-1]=3;
					} else {
						Vote3[i5+i2-1]=2;
						Vote4[i5+i2-1]=2;
						i8++;
					}
				}
			}

			Value3[N3]=Value2[i1];
			Indif4[N3]=i8;
			cool4 [N3]=j2;
		}
	}

	N3++;

	N4=N3;

	for (i1=0; i1<N4; i1++) {
		Value4[i1]=Value3[i1]+0.0;
	}

	for (i1=0; i1<M; i1++) {
		completed[i1]=false;
	}

	SimplexNoetig=true;
	PropCompletionSimple();
	EKarpSimple();

	i99=0;

	while (SimplexNoetig==true) {
		PropCompletion();
		completed[tobecompleted]=true;
		i99++;
		EKarp();
	}
}

/*******************************************************************************/

void Elimination() {
	int i1;

	for (i1=0; i1<=M; i1++) {
		g2=i1;
		VoteMana();

		/*datafile=fopen("output.txt","a+");
		fprintf(datafile,"%f",Output1);*/

		Matrix2[(M+1)*g3+i1]=Output1;

		/*if (i1==M)
		fprintf(datafile,"\n");
		else
		fprintf(datafile," ");

		fclose(datafile);*/
	}
}

/*******************************************************************************/

void Calculation_of_the_Strengths_of_the_Vote_Managements() {
	int i1,i2,i3,i4;
	bool j1;

	i1=N2;

	if (M+1>N2) {
		i1=M+1;
	}

	Length4      =i1;
	Length6      =i1;
	Length7      =i1;
	LengthArc    =i1;
	LengthArc7   =i1;
	LengthArc8   =i1;
	LengthVertex =i1;
	LengthVertex7=i1;
	LengthVertex8=i1;
	LengthIndiff7=i1;
	LengthIndiff8=i1;

	Kombi    =(int                 *)calloc(C+2,sizeof(int));
	Matrix1  =(int                 *)calloc(Comb2*(M+1)+1,
			sizeof(int));
	Matrix2  =(long double         *)calloc(Comb2*(M+1)+1,
			sizeof(long double));
	Vote3    =(unsigned char       *)calloc(N2*M+1,sizeof(unsigned char));
	Value3   =(int                 *)calloc(N2+1,sizeof(int));
	Vote4    =(unsigned char       *)calloc(Length4*M+1,
			sizeof(unsigned char));
	Value4   =(long double         *)calloc(Length4+1,
			sizeof(long double));
	Indif4   =(int                 *)calloc(Length4+1,
			sizeof(int));
	cool4    =(bool                *)calloc(Length4+1,
			sizeof(bool));
	Kombi4   =(int                 *)calloc(M+1,sizeof(int));
	Vote5    =(unsigned char       *)calloc(i1*M+1,sizeof(unsigned char));
	Value5   =(int                 *)calloc(i1+1,sizeof(int));
	Test     =(unsigned char       *)calloc(M+1,sizeof(unsigned char));
	Test3    =(int                 *)calloc(N2+1,sizeof(int));
	Indiff   =(struct IndiffElement*)calloc(Length6+1,
			sizeof(struct IndiffElement));
	Arcs     =(struct ArcElement   *)calloc(LengthArc+1,
			sizeof(struct ArcElement));
	Arcs7    =(struct ArcElement   *)calloc(LengthArc7+1,
			sizeof(struct ArcElement));
	Arcs8    =(struct ArcElement   *)calloc(LengthArc8+1,
			sizeof(struct ArcElement));
	Vertices =(struct VertexElement*)calloc(LengthVertex+1,
			sizeof(struct VertexElement));
	Vertices7=(struct VertexElement*)calloc(LengthVertex7+1,
			sizeof(struct VertexElement));
	Vertices8=(struct VertexElement*)calloc(LengthVertex8+1,
			sizeof(struct VertexElement));
	Votes    =(struct VotesElement *)calloc(M+1,sizeof(struct VotesElement));
	Votes1   =(int                 *)calloc(M+1,sizeof(int));
	completed=(bool                *)calloc(M+1,sizeof(bool));
	Vote7    =(bool                *)calloc(Length7*M+1,
			sizeof(bool));
	Value7   =(long double         *)calloc(Length7+1,
			sizeof(long double));
	Vote8    =(bool                *)calloc(Length7*M+1,
			sizeof(bool));
	Value8   =(long double         *)calloc(Length7+1,
			sizeof(long double));
	Indiff7  =(struct IndiffElement*)calloc(LengthIndiff7+1,
			sizeof(struct IndiffElement));
	Indiff8  =(struct IndiffElement*)calloc(LengthIndiff8+1,
			sizeof(struct IndiffElement));

	/*datafile=fopen("output.txt","w+");
	fclose(datafile);*/

	i4=0;

	for (i1=0; i1<=M; i1++) {
		Kombi[i1]=i1;
	}

	/*datafile=fopen("output.txt","a+");

	fprintf(datafile,"%d ",i4+1);*/

	for (i1=0; i1<=M; i1++) {
		g1=Kombi[i1];
		Matrix1[(M+1)*i4+i1]=g1;
		//Print1();
	}

//fclose(datafile);

	g3=i4;
	Elimination();

	i4++;

	j1=true;

	while (j1==true) {
		j1=false;

		for (i1=M; (j1==false) && (i1>=0); i1--)
			if (Kombi[i1]!=C-M+i1-1) {
				j1=true;

				Kombi[i1]++;

				i3=1;
				for (i2=i1+1; i2<=M; i2++) {
					Kombi[i2]=Kombi[i1]+i3;
					i3++;
				}
			}

		if (j1==true) {
			//datafile=fopen("output.txt","a+");

			//fprintf(datafile,"%d ",i4+1);

			for (i1=0; i1<=M; i1++) {
				g1=Kombi[i1];
				Matrix1[(M+1)*i4+i1]=g1;
				//Print1();
			}

			//fclose(datafile);

			g3=i4;
			Elimination();

			i4++;
		}
	}

	free(Kombi);
	free(Vote3);
	free(Value3);
	free(Vote4);
	free(Value4);
	free(Indif4);
	free(cool4);
	free(Kombi4);
	free(Vote5);
	free(Value5);
	free(Test);
	free(Test3);
	free(Indiff);
	free(Arcs);
	free(Arcs7);
	free(Arcs8);
	free(Vertices);
	free(Vertices7);
	free(Vertices8);
	free(Votes);
	free(Votes1);
	free(completed);
	free(Vote7);
	free(Value7);
	free(Vote8);
	free(Value8);
	free(Indiff7);
	free(Indiff8);
}

/*******************************************************************************/

void Kombinationen() {
	int  i1,i2,i3,i4;
	bool j1;
	int *KombiG;

	Sets.resize(Comb1*M+1);
	KombiG=(int*)calloc(M+1,sizeof(int));

	for (i1=0; i1<M; i1++) {
		KombiG[i1]=i1;
	}

	for (i1=0; i1<M; i1++) {
		Sets[Comb1*i1]=i1;
	}

	for (i1=1; i1<Comb1; i1++) {
		j1=true;

		for (i2=M-1; (j1==true) && (i2>=0); i2--)
			if (KombiG[i2]!=C-M+i2) {
				j1=false;

				KombiG[i2]=KombiG[i2]+1;

				i4=1;

				for (i3=i2+1; i3<M; i3++) {
					KombiG[i3]=KombiG[i2]+i4;
					i4++;
				}
			}

		for (i2=0; i2<M; i2++) {
			Sets[i1+Comb1*i2]=KombiG[i2];
		}
	}

	free(KombiG);
}

/*******************************************************************************/

void Kombinationen2() {
	/* This procedure generates the digraph where the C!/((M!*(C-M)!) vertices    */
	/* represent the possible combinations to choose M candidates out of          */
	/* C candidates. Link[i].Value is the strength of the link from vertex        */
	/* Link[i].winner to vertex Link[i].loser.                                    */

	int i1,i2,i3,i4,i5,i6,i7;
	//int *KombG1,*KombG2;
	long double d1;

	m1=Comb2*(M+1)*M;

	std::vector<int> KombG1(M+2), KombG2(M+2);
	Link   =(struct DefeatElement*)calloc(m1+1,sizeof(struct DefeatElement));
	Zeilenerster =(int*)calloc(Comb1+1,sizeof(int));
	Spaltenerster=(int*)calloc(Comb1+1,sizeof(int));

	for (i1=0; i1<Comb1; i1++) {
		Zeilenerster [i1]=-1;
		Spaltenerster[i1]=-1;
	}

	i1=0;

	for (i2=0; i2<Comb2; i2++) {
		i3=(M+1)*i2;

		for (i4=0; i4<=M; i4++) {
			KombG1[i4]=Matrix1[i3+i4];
		}

		for (i4=0; i4<=M; i4++) {
			i5=Comb1-1;

			for (i6=0; i6<i4; i6++) {
				i5=i5-over(C-KombG1[i6]-1,M-i6);
			}

			for (i6=i4+1; i6<=M; i6++) {
				i5=i5-over(C-KombG1[i6]-1,M-(i6-1));
			}

			KombG2[i4]=i5;
		}

		for (i4=0; i4<=M; i4++) {
			i5=KombG2[i4];
			d1=Matrix2[i3+i4];

			for (i6=0; i6<=M; i6++)
				if (i4!=i6) {
					i7=KombG2[i6];

					Link[i1].winner  =i5;
					Link[i1].loser   =i7;

					Link[i1].Value   =d1;

					Link[i1].RN      =Zeilenerster[i5];
					Zeilenerster[i5] =i1;

					Link[i1].UN      =Spaltenerster[i7];
					Spaltenerster[i7]=i1;

					i1++;
				}
		}
	}

	free(Matrix1);
	free(Matrix2);
}

/*******************************************************************************/

council_t Dijkstra() {
	int i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,erster,letzter;
	int *pairwise,*p1,*p2,*rank;
	long double d1,d2;
	bool j1,j2,j3;
	bool *marked;

	council_t winner; // -KM

	std::vector<long double> Von(Comb1+1), Nach(Comb1+1);
	std::vector<bool> Richtig(Comb1+1);
	std::vector<int> ON(Comb1+1), UN(Comb1+1);

	for (i1=0; i1<Comb1; i1++) {
		Richtig[i1]=true;
	}

	for (i1=0; i1<Comb1; i1++) {
		if (Richtig[i1]==true) {
			for (i2=0; i2<Comb1; i2++) {
				Von   [i2]=-1.0;
				ON    [i2]=i2-1;
				UN    [i2]=i2+1;
			}

			if (i1==0) {
				ON[UN[i1]]=ON[i1];
			} else {
				if (i1==Comb1-1) {
					UN[ON[i1]]=UN[i1];
				}

				else {
					UN[ON[i1]]=UN[i1];
					ON[UN[i1]]=ON[i1];
				}
			}

			erster=0;
			if (i1==0) {
				erster=1;
			}

			letzter=Comb1-1;
			if (i1==Comb1-1) {
				letzter=Comb1-2;
			}

			d1=-1.0;

			i2=Zeilenerster[i1];
			while (i2!=-1) {
				d2=Link[i2].Value;
				Von[Link[i2].loser]=d2;
				i2=Link[i2].RN;

				if (d2>d1) {
					d1=d2;
				}
			}

			for (i2=0; i2<Comb1-1; i2++) {
				i3=erster;
				d2=-1.0;

				while ((i3<Comb1) && (d2<d1)) {
					if (Von[i3]>d2) {
						i4=i3;
						d2=Von[i3];
					}

					i3=UN[i3];
				}

				d1=d2;

				if (i4==erster) {
					erster=UN[i4];
					ON[UN[i4]]=ON[i4];
				} else {
					if (i4==letzter) {
						letzter=ON[i4];
						UN[ON[i4]]=UN[i4];
					}

					else {
						UN[ON[i4]]=UN[i4];
						ON[UN[i4]]=ON[i4];
					}
				}

				i3=Zeilenerster[i4];
				while (i3!=-1) {
					i5=Link[i3].loser;

					d2=Link[i3].Value;
					if (d2>d1) {
						d2=d1;
					}

					if (d2>Von[i5]) {
						Von[i5]=d2;
					}

					i3=Link[i3].RN;
				}
			}

			for (i2=0; i2<Comb1; i2++) {
				Nach  [i2]=-1.0;
				ON    [i2]=i2-1;
				UN    [i2]=i2+1;
			}

			if (i1==0) {
				ON[UN[i1]]=ON[i1];
			} else {
				if (i1==Comb1-1) {
					UN[ON[i1]]=UN[i1];
				}

				else {
					UN[ON[i1]]=UN[i1];
					ON[UN[i1]]=ON[i1];
				}
			}

			erster=0;
			if (i1==0) {
				erster=1;
			}

			letzter=Comb1-1;
			if (i1==Comb1-1) {
				letzter=Comb1-2;
			}

			d1=-1.0;

			i2=Spaltenerster[i1];
			while (i2!=-1) {
				d2=Link[i2].Value;
				Nach[Link[i2].winner]=d2;
				i2=Link[i2].UN;

				if (d2>d1) {
					d1=d2;
				}
			}

			for (i2=0; i2<Comb1-1; i2++) {
				i3=erster;
				d2=-1.0;

				while ((i3<Comb1) && (d2<d1)) {
					if (Nach[i3]>d2) {
						i4=i3;
						d2=Nach[i3];
					}

					i3=UN[i3];
				}

				d1=d2;

				if (i4==erster) {
					erster=UN[i4];
					ON[UN[i4]]=ON[i4];
				} else {
					if (i4==letzter) {
						letzter=ON[i4];
						UN[ON[i4]]=UN[i4];
					}

					else {
						UN[ON[i4]]=UN[i4];
						ON[UN[i4]]=ON[i4];
					}
				}

				i3=Spaltenerster[i4];
				while (i3!=-1) {
					i5=Link[i3].winner;

					d2=Link[i3].Value;
					if (d2>d1) {
						d2=d1;
					}

					if (d2>Nach[i5]) {
						Nach[i5]=d2;
					}

					i3=Link[i3].UN;
				}
			}

			for (i2=0; i2<Comb1; i2++)
				if (i2!=i1) {
					if (Von[i2]>Nach[i2]+eps4) {
						Richtig[i2]=false;
					}

					if (Von[i2]+eps4<Nach[i2]) {
						Richtig[i1]=false;
					}
				}
		}
	}

	free(Link);
	free(Spaltenerster);
	free(Zeilenerster);

	/* Now, we check how many potential winning sets there are. */

	i1=0;
	i2=0;

	for (i3=0; i3<Comb1; i3++)
		if (Richtig[i3]==true) {
			if (i1==0) {
				i2=i3;
				i4=i3;
			} else {
				UN[i4]=i3;
				i4=i3;
			}

			i1++;
		}

	UN[i4]=Comb1;

	/* If there is only one potential winning set, then this */
	/* potential winning set is the unique winning set.      */

	if (i1==1) {
		// Return the winning set -KM

		for (i3 = 0; i3 < M; ++i3) {
			winner.push_back(Sets[i2 + Comb1 * i3]);
		}

		return (winner);
	}

	/* If there is more than one potential winning set, then we calculate the     */
	/* Schulze relation (as defined in (2.2.2.4) of the paper "A New Monotonic,   */
	/* Clone-Independent, Reversal Symmetric, and Condorcet-Consistent            */
	/* Single-Winner Election Method") and eliminate potential winning sets       */
	/* (as defined at stage 2 of section 5.4 of the paper "Free Riding and Vote   */
	/* Management under Proportional Representation by the Single Transferable    */
	/* Vote").                                                                    */

	else {
		pairwise=(int*)calloc(C*C+1,sizeof(int));

		for (i3=0; i3<C; i3++)
			for (i4=0; i4<C; i4++) {
				pairwise[C*i3+i4]=0;
			}

		for (i3=0; i3<N2; i3++)
			for (i4=0; i4<C-1; i4++)
				for (i5=i4+1; i5<C; i5++) {
					i6=Vote2[C*i3+i4];
					i7=Vote2[C*i3+i5];

					if (i6<i7) {
						pairwise[C*i4+i5]=pairwise[C*i4+i5]+Value2[i3];
					} else if (i6>i7) {
						pairwise[C*i5+i4]=pairwise[C*i5+i4]+Value2[i3];
					}
				}

		p1=(int*)calloc(C*C+1,sizeof(int));
		p2=(int*)calloc(C*C+1,sizeof(int));

		for (i3=0; i3<C; i3++)
			for (i4=0; i4<C; i4++) {
				p1[C*i3+i4]=pairwise[C*i3+i4];
				p2[C*i3+i4]=pairwise[C*i4+i3];
			}

		free(pairwise);

		/* We use >ratio (as defined in section 2.1 of the paper "A New Monotonic, */
		/* Clone-Independent, Reversal Symmetric, and Condorcet-Consistent         */
		/* Single-Winner Election Method") as a measure for the strength of a      */
		/* pairwise link, because >ratio corresponds to proportional completion    */
		/* (as defined in section 5.3 of the paper "Free Riding and Vote Management*/
		/* under Proportional Representation by the Single Transferable Vote").    */

		for (i3=0; i3<C; i3++)
			for (i4=0; i4<C; i4++)
				if (i3!=i4)
					for (i5=0; i5<C; i5++)
						if (i3!=i5)
							if (i4!=i5) {
								i6=p1[C*i4+i3];
								i7=p2[C*i4+i3];

								i8=p1[C*i3+i5];
								i9=p2[C*i3+i5];

								if ((i6*i9>i7*i8)
									|| ((i6> i8) && (i7<=i9))
									|| ((i6>=i8) && (i7< i9))
									|| ((i6> i7) && (i8<=i9))
									|| ((i6>=i7) && (i8< i9))) {
									i10=i8;
									i11=i9;
								} else {
									i10=i6;
									i11=i7;
								}

								i12=p1[C*i4+i5];
								i13=p2[C*i4+i5];

								if ((i10*i13>i11*i12)
									|| ((i10> i12) && (i11<=i13))
									|| ((i10>=i12) && (i11< i13))
									|| ((i10> i11) && (i12<=i13))
									|| ((i10>=i11) && (i12< i13))) {
									p1[C*i4+i5]=i10;
									p2[C*i4+i5]=i11;
								}
							}

		marked=(bool*)calloc(C+1,sizeof(bool));
		rank  =(int *)calloc(C+1,sizeof(int));

		for (i3=0; i3<C; i3++) {
			marked[i3]=false;
		}

		i3=0;
		j1=true;

		while (j1==true) {
			j1=false;

			for (i4=0; (i4<C) && (j1==false); i4++)
				if (marked[i4]==false) {
					j2=true;

					for (i5=0; i5<C; i5++)
						if (marked[i5]==false)
							if (i4!=i5) {
								i6=p1[C*i4+i5];
								i7=p2[C*i4+i5];
								i8=p1[C*i5+i4];
								i9=p2[C*i5+i4];

								if ((i6*i9>i7*i8)
									|| ((i6> i8) && (i7<=i9))
									|| ((i6>=i8) && (i7< i9))
									|| ((i6> i7) && (i8<=i9))
									|| ((i6>=i7) && (i8< i9)))

								{
									j3=true;
								} else {
									j2=false;
								}
							}

					if (j2==true) {
						j1=true;
						rank[i3]=i4;
						marked[i4]=true;
						i3++;
					}
				}
		}

		for (i4=0; i4<i3; i4++) {
			j1=false;

			i5=i2;

			while (i5<Comb1) {
				if (Richtig[i5]==true)
					for (i6=0; i6<M; i6++)
						if (Sets[i5+Comb1*i6]==rank[i4]) {
							j1=true;
						}

				i5=UN[i5];
			}

			if (j1==true) {
				i5=i2;

				while (i5<Comb1) {
					if (Richtig[i5]==true) {
						j2=false;

						for (i6=0; i6<M; i6++)
							if (Sets[i5+Comb1*i6]==rank[i4]) {
								j2=true;
							}

						if (j2==false) {
							Richtig[i5]=false;
						}
					}

					i5=UN[i5];
				}
			}
		}

		/*datafile=fopen("output.txt","a+");
		fprintf(datafile,"\n");*/

		i5=i2;

		bool done = false;

		while (i5<Comb1) {
			if (Richtig[i5]==true) {
				// Ditto -KM
				// But there may be multiple winners here! Check later.

				for (i6 = 0; i6 < M && !done; ++i6) {
					winner.push_back(Sets[i5 + Comb1 * i6]);
				}

				done = true;
			}

			i5=UN[i5];
		}

		//fclose(datafile);

		free(p1);
		free(p2);
		free(marked);
		free(rank);
	}

	return (winner);
}

/*******************************************************************************/



/*int main()
{
 Reading_the_Input();
 Analyzing_the_Input();
 Comb1=over(C,M);
 Comb2=over(C,M+1);
 Calculation_of_the_Strengths_of_the_Vote_Managements();
 Kombinationen();
 Kombinationen2();
 council_t winners = Dijkstra();

 cout << "And the winners are: ";
 print3(winners);
 cout << endl;

 time(&finish);
 time2=clock();

 elapsed_time=difftime(finish,start);

 printf("runtime=%d seconds (%d processor timer ticks)\n",elapsed_time,time2-time1);

}*/

#pragma GCC diagnostic pop

council_t SchulzeSTV::get_council(size_t council_size,
	size_t num_candidates,
	const election_t & ballots) const {

	if (council_size > num_candidates) {
		throw std::invalid_argument("Schulze STV: Too few candidates "
			"for council.");
	}

	// Num seats = num candidates is not supported by Schulze STV.
	if (council_size == num_candidates) {
		council_t full_council(num_candidates, 0);
		std::iota(full_council.begin(), full_council.end(), 0);
		return full_council;
	}

	read_ballot_input(ballots, council_size, num_candidates);
	Analyzing_the_Input();
	// Really large numbers, and they're used for allocation!
	Comb1 = over(C, M);
	Comb2 = over(C, M+1);
	Calculation_of_the_Strengths_of_the_Vote_Managements();
	Kombinationen();
	Kombinationen2();
	council_t council = Dijkstra();
	//cout << "DEBUG: Winners are ";
	//print3(council);
	return council;
}

void SchulzeSTV::print_schulze_stv_prefs(size_t council_size,
	size_t num_candidates, const election_t & ballots) const {

	std::cerr << "M " << council_size << "\n";
	std::cerr << "C " << num_candidates << "\n";
	std::cerr << "N " << ballots.size() << "\n";
	std::cerr << "F 2" << "\n"; // format type

	std::cerr << "\nBEGIN\n";

	int voter = 1;

	for (election_t::const_iterator pos = ballots.begin();
		pos != ballots.end(); ++pos) {
		std::cerr << voter++ << " ";
		for (ordering::const_iterator opos = pos->contents.begin();
			opos != pos->contents.end(); ++opos) {
			std::cerr << (char)('A' + opos->get_candidate_num()) << " ";
		}
		std::cerr << "\n";
	}
	std::cerr << "END" << std::endl;
}
