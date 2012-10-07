#!/bin/bash
../../scripts/runtests.py -s ./ -g ../gcov_files/$1/ -t ./test/ -x ../../scripts/
../../scripts/lcov.py -g ../gcov_files/ -o ../../docs/lcov -r _OSByteOrder.h -r _string.h
