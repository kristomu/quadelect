// CPO-STV is a Condorcetian method where, instead of candidates, the
// comparisons are done on sets. On a comparison between two sets:
//
// - Eliminate the candidates that aren't in either outcome.
// - Transfer surplus votes from candidates above quota (after this step, all
// 	candidates should be <= quota)
// - The votes for one side of the outcome equal the sum of the scores of the
// 	candidates on that side.

// Don't bother with Meek yet.

// The trick here isn't to implement this, which'll be fairly easy, but to
// make a framework class for these kinds of methods (set-based comparison
// methods). If we get that working, perhaps we can have Schulze STV easily
// implemented -- if I understand how it works. But note that you can hillclimb
// Schulze STV, but not CPO-STV. CFPRM would also go easily if we get this
// framework, err, working.
