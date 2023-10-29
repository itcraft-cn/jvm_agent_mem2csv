#!/bin/bash

CURR_DIR=$(pwd)
#LIB_PATH=$CURR_DIR/build/linux/x86_64/debug
LIB_PATH=$CURR_DIR/build/linux/x86_64/release

cd jar_test || exit 1

export JAVA_HOME="$JAVA_HOME8_IBM"
export PATH="$JAVA_HOME/bin":"$PATH"
export LD_LIBRARY_PATH="$LIB_PATH":"$LD_LIBRARY_PATH"
export JAVA_OPTS="-Xmx2G -Xms2G"

echo normal call
time java $JAVA_OPTS -jar tiny-rules-sample.jar

echo call with agent
time java $JAVA_OPTS -agentlib:j9objrecord -jar tiny-rules-sample.jar
#time valgrind java $JAVA_OPTS -agentlib:j9objrecord -jar tiny-rules-sample.jar
