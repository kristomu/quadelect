#!/bin/sh

echo Setting up pre-commit hook...

cd .git/hooks/
ln -s ../../hooks/pre-commit ./pre-commit
