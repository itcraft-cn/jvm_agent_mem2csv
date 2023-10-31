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

if [ -e "$HOME/.bashrc" ]; then
    source "$HOME/.bashrc"
fi

CURR_DIR=$(pwd)

DEBUG_BINARY_PATH=$CURR_DIR"/build/linux/x86_64/debug"
RELEASE_BINARY_PATH=$CURR_DIR"/build/linux/x86_64/release"

compile debug
compile release

export JAVA_HOME="$JAVA_HOME8_IBM"
export PATH="$JAVA_HOME/bin":"$PATH"

javac -sourcepath java_src -d java_classes java_src/demo/A.java

#echo normal call
#time java -cp java_classes:"$CLASSPATH" demo.A

OLD_LD_PATH=$LD_LIBRARY_PATH

echo call with agent, debug
time LD_LIBRARY_PATH=$DEBUG_BINARY_PATH:$OLD_LD_PATH java -agentlib:j9objrecord -cp java_classes:"$CLASSPATH" demo.A

echo call with agent, release
time LD_LIBRARY_PATH=$RELEASE_BINARY_PATH:$OLD_LD_PATH java -agentlib:j9objrecord -cp java_classes:"$CLASSPATH" demo.A
