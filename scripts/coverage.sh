#!/bin/bash
../../scripts/runtests.py -s ./ -g ../gcov_files/ -t ./test/ -x ../../scripts/
../../scripts/lcov.py -g ../gcov_files/ -o ../../docs/lcov
