#!/bin/sh

mkdir -p build
g++ -std=c++17 -O2 -g -o build/bootstrap-interpreter bootstrap-interpreter/unity_build.cpp
