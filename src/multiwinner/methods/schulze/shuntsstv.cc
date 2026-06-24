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
#include "common.h"

#include <numeric>
#include <vector>
#include <list>
#include <set>

#include "tools/tools.h"

/*******************************************************************************/

void SchulzeSTVCalc::Elimination() {
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

void SchulzeSTVCalc::Calculation_of_the_Strengths_of_the_Vote_Managements() {
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

void SchulzeSTVCalc::Kombinationen() {
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

void SchulzeSTVCalc::Kombinationen2() {
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
				i5=i5-choose(C-KombG1[i6]-1,M-i6);
			}

			for (i6=i4+1; i6<=M; i6++) {
				i5=i5-choose(C-KombG1[i6]-1,M-(i6-1));
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

council_t SchulzeSTVCalc::Dijkstra() {
	int i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,erster,letzter;
	int *pairwise,*p1,*p2,*rank;
	long double d1,d2;
	bool j1,j2;
	bool *marked;

	i4 = 0;

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

		return winner;
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
									//j3=true;
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

council_t SchulzeSTVCalc::analyze_and_get_outcome() {
	Analyzing_the_Input();
	// Really large numbers, and they're used for allocation!
	Comb1 = choose(C, M);
	Comb2 = choose(C, M+1);
	Calculation_of_the_Strengths_of_the_Vote_Managements();
	Kombinationen();
	Kombinationen2();
	council_t council = Dijkstra();

	return council;
}

/*******************************************************************************/



/*int main() {
	Reading_the_Input();
	Analyzing_the_Input();
	Comb1=choose(C,M);
	Comb2=choose(C,M+1);
	Calculation_of_the_Strengths_of_the_Vote_Managements();
	Kombinationen();
	Kombinationen2();
	council_t winners = Dijkstra();

	std::cout << "And the winners are: ";
	print3(winners);
	std::cout << endl;

	time(&finish);
	time2=clock();

	elapsed_time=difftime(finish,start);

	printf("runtime=%d seconds (%d processor timer ticks)\n",elapsed_time,
		time2-time1);

}*/

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

	SchulzeSTVCalc calc;

	calc.read_ballot_input(ballots, council_size, num_candidates);

	return calc.analyze_and_get_outcome();
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
			// TODO: check for equal rank, which is disallowed...
			std::cerr << (char)('A' + opos->get_candidate_num()) << " ";
		}
		std::cerr << "\n";
	}
	std::cerr << "END" << std::endl;
}
