Method design tests.

For now, I'll try to create a program to determine if resistant set
compliance and monotonicity is possible for four candidates with a
certain simplification of state space; and before I do that, I'll
try to get it running for three candidates.

The idea is as follows:

For Copeland, we don't actually need to know the magnitudes of
pairwise defeats, just whether A beat B or vice versa, to determine
the outcome. (That's if we disregard pairwise ties, which we
probably can with some handwaving.) So do the same with the sub-
elections in the resistant set:

For each sub-election of cardinality k, and disregarding exact ties
as above, at least one and at most (k-1) of the candidates exceed the
"quota" of 1/k. We can thus create a "fingerprint" by encoding the
list of candidates exceeding the quota for each sub-election.

For instance, for three-candidate elections, the subelections are:

-	{A, B, C}	2^3-2 = 6 options
-	{A, B}		2^2-2 = 2 options
-	{A, C}		2 options
-	{B, C}		2 options

which gives 48 possibilities in all. (For four candidates, I think
the total number of distinct election types of this sort is around
a million, but I'd have to check.)

Then we'd like to assign a set of winners to each election type so
that the assignment both passes the resistant set criterion (never
picking someone outside the set) and is monotone.

One nice property of this way of dividing up elections is that it
doesn't matter how many voters there are (except for ties). So any
solution would hold for however many voters you want, apart from
ties starting to matter if you only have a few voters.

To assign a set of winners, we have absolute and relative constraints:

- No winner can come from outside the resistant set.
- If A is a winner here, A must be a winner in every election type
	that can be reached by a voter raising A one rank.
- If B is not a winner here but A is, then B must not be a winner
	in any election that can be reached by a voter raising
	A one rank.

The first criterion is obvious: it ensures resistant set compliance.
The second ensures that A doesn't lose when ranked higher. And the
third ensures that any way of breaking ties between the winners of
the election preserves monotonicity. (Suppose otherwise, that the
tiebreaking order is B>A>.., then if raising A makes B tie for first,
the winner after tiebreaking would change from A to B.)

Now consider this as a satisfiability problem, and let W\_e\_A be true
if A is a winner in the election with index e. (E.g. for the example
above, W\_1\_A being true means that A is a winner in election 1.)

The first constraint is easy: for any candidate X not in e's resistant
set, W\_e\_X = false.

For the second and third, let M(e, X) be the set of all election types
that can be reached by raising A one rank in some election that has
the type given by index e.

Then, the second implication is:
	`W\_e\_A ==> W\_m\_A for all m in M(e, A), for all candidates A,`

And the third is:
    `W\_e\_A and not W\_e\_B ==> not W\_m\_B for all m in M(e, A).`

I.e.
    `(not W\_e\_A) or W\_m\_A`
and
    `not (W\_e\_A and not W\_e\_B) or (not W\_m\_B),`

so in CNF we have
  `((not W\_e\_A) or W\_m\_A)` and
  `((not W\_e\_A) or W\_e\_B or (not W\_m\_B))`

If these constraints can be fulfilled (which can be determined by a
SAT solver), then there exists a solution, otherwise not.
