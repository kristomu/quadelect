Miscellaneous cardinal and Approval-based methods go here.

As Quadelect doesn't natively support approval-style ballot generation, the
methods that make use of approval data and cutoffs generate 0-1 cardinal
ballots by means of a filter passed to them. The most common filter is the
mean utility threshold filter.

TODO later: properly check (here and in filters) that the input ballots
actually contain ratings.
