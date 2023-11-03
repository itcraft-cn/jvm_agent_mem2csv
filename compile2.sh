#!/bin/bash

# compile/link by gcc directly
if [ -e build ]; then
    echo "dir [build] exists, creating is skipped"
else
    mkdir build
fi

gcc -DJ9 -c -O3 -fPIC -I$JAVA_HOME8_IBM/include -o build/main.o src/main.c
gcc -shared -fPIC -o build/libj2mem2csv.so build/main.o
gcc -c -O3 -fPIC -I$JAVA_HOME8/include -I$JAVA_HOME8/include/linux -o build/main.o src/main.c
gcc -shared -fPIC -o build/libhsmem2csv.so build/main.o
rm -rf build/main.o
