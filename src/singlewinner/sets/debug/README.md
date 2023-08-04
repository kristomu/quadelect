This directory contains an alternate implementation of the Landau set,
using (adapted) code described by Kevin Venzke.

I was concerned that my Landau set implementation was faulty, but I couldn't
find any disagreements between it and the Venzke implementation here, which
suggests that they're both working correctly.

As the Venzke Landau implementaion exists mainly for debug purposes, it's not
available through get_singlewinner_methods and must be declared manually if
it's to be used.
