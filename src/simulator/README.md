This directory contains "simulators". A simulator is a class that in some way
evaluates a voting method and returns a quality score or penalty. For instance,
a simulator might test a method's manipulation resistance or its social utility
efficiency (VSE).

A simulator may be used directly or as part of a multi-armed bandit search to
find the best method by that simulator's embodied objective. In the latter
case, the simulator must implement a linearized scoring function. This
linearized function depends nonlinearly only on constants that stay the same
regardless of the method being tested. For instance, this can be the expected
utility by the voters of the highest utility candidate drawn by the model. See
the VSE class for an example.

Alternatively, the simulator can simply not implement the linearization, but
the multi-armed bandit logic will exclude these simulators from testing because
the search can't be guaranteed to find the optimum.

Each simulator implements the following functionality:
	- Simulate: Do one round of simulation and return the resulting score,
		exactly if requested.
	- Return mean result: returns the mean result so far.
	- Return name: returns the display name of the simulation. This usually
		includes the election method being tested.
