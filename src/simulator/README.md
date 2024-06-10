This directory contains "simulators". A simulator is a class that in some way
evaluates a voting method and returns a quality score. For instance, a
simulator might test a method's manipulation resistance or its social utility
efficiency (VSE).

Every simulator implemented so far is linear, which is a requirement for the
multi-armed bandit search to be able to use them. Linearity means that
evaluating the voting method over n rounds returns the same score
as the mean score for each of the n evaluations.

To make VSE linear, we have to perform a trick which makes sense with a large
number of rounds - see the VSE source.

Each simulator implements the following functionality:
	- Set a scale factor: Uses the provided objects (usually a ballot
		generator) to determine a scaling factor that is used to scale
		the output score properly. Since this is a constant, it doesn't
		affect linearity.
	- Simulate: Do one round of simulation and return the resulting score,
		multiplied by the scale factor.
	- Return mean result: returns the mean result so far.
	- Return name: returns the display name of the simulation. This usually
		includes the election method being tested.
	- Return minimum and maximum values: the score must be bounded. Note:
		these values are post-scaling.
