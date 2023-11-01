#!/bin/bash

function compile(){
    xmake config -m $1
    if xmake
    then
        echo compile successfully
    else
        echo compile failed
        exit 1
    fi
}

compile debug
compile release

# compile/link by gcc direct
#gcc -c -O3 -fPIC -I -o main.o main.c
#gcc -shared -fPIC -o libx.so main.o

