#!/bin/sh

mkdir -p build
# g++ -Wall -Wextra -std=c++17 -O2 -g -o build/bootstrap-interpreter bootstrap-interpreter/unity_build.cpp
g++ -Wall -Wextra -std=c++17 -g -o build/bootstrap-interpreter bootstrap-interpreter/unity_build.cpp
