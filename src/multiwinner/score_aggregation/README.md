This directory contains classes for aggregating scores for optimizing
(exhaustive) PR methods. These methods generally come in two variants:
utilitarian, where highest total wins, and egalitarian, where highest lowest
score wins. To handle both with the same infrastructure, we need a somewhat
substantial class for determining if one set of scores is better (greater
minimum) than another; and a much simpler class with the same interface for
determining if the sum of one set of scores is better than another.
