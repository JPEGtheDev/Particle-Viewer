#!/usr/bin/env sh
#gcc build script
mkdir build;
cd build;
/home/travis/build/JPEGtheDev/cmake/bin/cmake ..;
if [ ${COVERITY_SCAN_BRANCH} != 1 ]; then make ; fi
