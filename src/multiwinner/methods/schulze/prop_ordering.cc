/*   (c) Markus Schulze, 2007                                                  */
/*   markus.schulze@alumni.tu-berlin.de                                        */
/*   draft, 24 September 2007                                                  */
/*                                                                             */
/* This program has been written in Microsoft Visual C++ 5.0.                  */
/*                                                                             */
/* This program calculates the Schulze proportional ranking with proportional  */
/* completion, as defined in this series of papers:                            */
/*                                                                             */
/*              http://m-schulze.webhop.net/schulze1.pdf                       */
/*              http://m-schulze.webhop.net/schulze2.pdf                       */
/*              http://m-schulze.webhop.net/schulze3.zip                       */

// Plus another shunt for quadelect purposes, even though it's pretty ugly. -KM

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "prop_ordering.h"

#include <numeric>

void SchulzePOCalc::Elimination() {
	int i1;

	for (i1=0; i1<=M; i1++) {
		if ((elected[Kombi[i1]]==false) && (Kombi[i1]!=g3)) {
			g2=i1;
			VoteMana();
			Matrix2[C*g3+Kombi[i1]]=Output1;
		} else {
			Matrix2[C*g3+Kombi[i1]]=0.0;
		}
	}
}

size_t SchulzePOCalc::Filling_the_next_place() {
	int i1,i2,i3,i4;
	long double d1;
	bool j1,j2;

	bool *marked,*winner;
	long double *p;

	M++;

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
	Matrix2  =(long double         *)calloc(C*C+1,sizeof(long double));
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

	marked   =(bool*)calloc(M+2,sizeof(bool));
	p        =(long double*)calloc(C*C+1,sizeof(long double));
	winner   =(bool*)calloc(C+1,sizeof(bool));

	for (i1=0; i1<C-1; i1++) {
		if (elected[i1]==false) {
			for (i2=i1+1; i2<C; i2++) {
				if (elected[i2]==false) {
					Kombi[0]=i1;
					Kombi[1]=i2;

					i3=2;
					for (i4=0; i4<C; i4++)
						if (elected[i4]==true) {
							Kombi[i3]=i4;
							i3++;
						}

					g3=i1;
					Elimination();

					g3=i2;
					Elimination();
				} else {
					Matrix2[C*i1+i2]=0.0;
					Matrix2[C*i2+i1]=0.0;
				}
			}
		}

		else
			for (i2=0; i2<C; i2++) {
				Matrix2[C*i1+i2]=0.0;
				Matrix2[C*i2+i1]=0.0;
			}
	}

	for (i1=0; i1<C; i1++)
		if (elected[i1]==false)
			for (i2=0; i2<C; i2++)
				if (i1!=i2)
					if (elected[i2]==false) {
						p[C*i1+i2]=Matrix2[C*i1+i2];
					}

	for (i1=0; i1<C; i1++)
		if (elected[i1]==false)
			for (i2=0; i2<C; i2++)
				if (i1!=i2)
					if (elected[i2]==false)
						for (i3=0; i3<C; i3++)
							if (i1!=i3)
								if (i2!=i3)
									if (elected[i3]==false) {
										d1=p[C*i2+i1];
										if (p[C*i1+i3]<d1) {
											d1=p[C*i1+i3];
										}

										if (p[C*i2+i3]<d1) {
											p[C*i2+i3]=d1;
										}
									}

	if (M==1)
		for (i1=0; i1<C; i1++)
			for (i2=0; i2<C; i2++)
				if (i1!=i2) {
					path[C*i1+i2]=p[C*i1+i2];
				}

	for (i1=0; i1<C; i1++)
		if (elected[i1]==false) {
			winner[i1]=true;
		}

	for (i1=0; i1<C; i1++)
		if (elected[i1]==false)
			for (i2=0; i2<C; i2++)
				if (i1!=i2)
					if (elected[i2]==false)
						if (p[C*i2+i1]>p[C*i1+i2]+eps4) {
							winner[i1]=false;
						}

	/*for (i1=0; i1<C; i1++)
		if (elected[i1]==false) {
			g1=i1;

			std::cout << (char)('A' + g1) << " ";

			for (i2=0; i2<C; i2++)
				if (elected[i2]==false) {
					std::cout << Matrix2[C*i1+i2] << " ";
				}

			std::cout << "\n";
		}
	*/

	j1=false;

	for (i1=0; i1<C; i1++) {
		if (elected[i1]==false)
			if (winner[i1]==true) {
				j2=true;

				for (i2=0; i2<C; i2++)
					if (elected[i2]==false)
						if (winner[i2]==true)
							if (i1!=i2)
								if (path[C*i2+i1]>path[C*i1+i2]+eps4) {
									j2=false;
								}

				if (j2==true) {
					if (j1==false) {
						i3=i1;

						elected[i3]=true;
						j1=true;

						//std::cout << "The " << M << ". place goes to candidate " << i1 << "\n";
						g1=i1;
					}
				}
			}
	}



	//fprintf(datafile,"\n");
	//fclose(datafile);

	free(Matrix2);

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

	free(marked);
	free(p);
	free(winner);

	return g1;
}

/*******************************************************************************/

council_t SchulzePOCalc::analyze_and_get_outcome() {
	int i1;

	elected = std::vector<bool>(C+1, false);

	for (i1=0; i1<C; i1++) {
		elected[i1]=false;
	}

	path = std::vector<long double>(C*C+1, 0);

	M=0;

	Analyzing_the_Input();

	council_t outcome;

	for (i1=0; i1<C-1; i1++) {
		outcome.push_back(Filling_the_next_place());
	}

	// The last candidate to be elected is whoever wasn't elected already.

	for (int i = 0; i < C; ++i) {
		if (!elected[i]) {
			outcome.push_back(i);
		}
	}

	return outcome;
}

council_t SchulzePOCalc::analyze_and_get_outcome(size_t council_size) {
	council_t outcome_in_order = analyze_and_get_outcome();

	if (council_size > outcome_in_order.size()) {
		throw std::invalid_argument("SchulzePOCalc: more seats than candidates!");
	}

	outcome_in_order.resize(council_size);

	return outcome_in_order;
}

council_t SchulzePropOrdering::get_ordered_council(
	size_t num_candidates, const election_t & ballots) const {

	SchulzePOCalc calc;

	calc.read_ballot_input(ballots, num_candidates, num_candidates);

	return calc.analyze_and_get_outcome();

}

council_t SchulzePropOrdering::get_council(size_t council_size,
	size_t num_candidates, const election_t & ballots) const {

	if (council_size > num_candidates) {
		throw std::invalid_argument("Schulze prop. ordering: Too few candidates "
			"for council.");
	}

	council_t outcome = get_ordered_council(
			num_candidates, ballots);

	outcome.resize(council_size);

	return outcome;
}