This directory contains filters, which are meta-methods that accept incoming
ballots and perform some kind of transformation on them to fit methods that
accept less data.

For instance, Approval voting only accepts two-level ballots and cardinal
ratings only accepts limited-slot ballots. Using a filter to quantize full-rank
ballots with real-valued ratings allows strategy tests to pass limited
ballot information to composite methods like STAR while still keeping track
of the voters' real preferences.

Composing methods like this is inelegant, but the alternatives are worse still.

TODO: Make filters able to interface with generators too, so that strategy
generators can generate ballots that _aren't_ normalized.
