#!/bin/bash
lcov -c -d ./ -o app_base.info --gcov-tool `pwd`/../tools/llvm-gcov.sh --include "/planeta-match-maker/PlanetaMatchMakerServer/source/*" --rc "lcov_branch_coverage=1" -i
ctest
lcov -c -d ./ -o app_test.info --gcov-tool `pwd`/../tools/llvm-gcov.sh --include "/planeta-match-maker/PlanetaMatchMakerServer/source/*" --rc "lcov_branch_coverage=1"
lcov -a app_base.info -a app_test.info -o app_total.info --rc "lcov_branch_coverage=1"
genhtml -o lcovHtml --num-spaces 4 -s --legend app_total.info --branch-coverage
