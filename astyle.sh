#!/bin/sh

# Java brace style, C/C++ code, forced tab indentation, 75 character
# lines, indent (not align) after parens, align references to middle,
# put braces around one-liners, indent classes and case switches,
# and keep one-line statements and if spacing to parens.

# Preserve the creation date, don't store an .orig backup, and only
# output the names of files that were formatted.

astyle -T -A2 -j -o --mode=c -xC75 -W2 -C -S -U -xU -H -Z --suffix=none -Q $*
