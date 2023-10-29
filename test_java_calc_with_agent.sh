#!/bin/bash

if [ -e "$HOME/.bashrc" ]; then
    source "$HOME/.bashrc"
fi

#DEBUG=0
DEBUG=1
CURR_DIR=$(pwd)

DEBUG_BINARY_PATH=$CURR_DIR"/build/linux/x86_64/debug"
RELEASE_BINARY_PATH=$CURR_DIR"/build/linux/x86_64/release"

if xmake
then
    echo compile successfully
else
    echo compile failed
    exit 1
fi

if [ "$DEBUG" -eq "1" ]; then
    LIB_PATH=$DEBUG_BINARY_PATH
else
    LIB_PATH=$RELEASE_BINARY_PATH
fi

export JAVA_HOME="$JAVA_HOME8_IBM"
export PATH="$JAVA_HOME/bin":"$PATH"
export LD_LIBRARY_PATH="$LIB_PATH":"$LD_LIBRARY_PATH"

javac -sourcepath java_src -cp java_classes:"$CLASSPATH" -d java_classes java_src/calc/A.java
javac -sourcepath java_src -cp java_classes:"$CLASSPATH" -d java_classes java_src/calc/B.java
javac -sourcepath java_src -cp java_classes:"$CLASSPATH" -d java_classes java_src/calc/C.java

echo normal call
time java -cp java_classes:"$CLASSPATH" calc.A

echo call with agent
time java -agentlib:j9objrecord -cp java_classes:"$CLASSPATH" calc.A

