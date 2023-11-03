#!/bin/bash

# compile/link by gcc direct
gcc -DJ9 -c -O3 -fPIC -I$JAVA_HOME8_IBM/include -o main.o src/main.c
gcc -shared -fPIC -o libj2mem2csv.so main.o
gcc -c -O3 -fPIC -I$JAVA_HOME8/include -I$JAVA_HOME8/include/linux -o main.o src/main.c
gcc -shared -fPIC -o libhsmem2csv.so main.o
rm -rf main.o
