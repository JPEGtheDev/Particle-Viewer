#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
cmake ..;
if [ ${COVERITY_SCAN_BRANCH} != 1 ]; then make ; fi
