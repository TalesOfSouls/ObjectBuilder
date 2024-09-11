#!/bin/bash

clear
mkdir -p ../build/objbuilder

#-Wno-strict-aliasing

g++ -Wall -O3 -std=c++11 -m64 \
    -Wno-unused-result \
    -march=native -mfpmath=sse -maes -msse4.2 -mavx -mavx2 -mavx512dq -mavx512f \
    -pthread \
    obj_builder.cpp \
    -o ../build/objbuilder/obj_builder