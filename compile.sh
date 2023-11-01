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

