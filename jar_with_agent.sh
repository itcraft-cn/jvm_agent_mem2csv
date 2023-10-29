#!/bin/bash

CURR_DIR=$(pwd)
#LIB_PATH=$CURR_DIR/target/debug
LIB_PATH=$CURR_DIR/target/release

cd jar_test || exit 1

export JAVA_HOME="$JAVA_HOME8"
export PATH="$JAVA_HOME/bin":"$PATH"
export LD_LIBRARY_PATH="$LIB_PATH":"$LD_LIBRARY_PATH"
export JAVA_OPTS="-Xmx2G -Xms2G"

echo normal call
time java $JAVA_OPTS -jar tiny-rules-sample.jar

echo call with agent
time java $JAVA_OPTS -agentlib:classmapdrawer -jar tiny-rules-sample.jar
#time valgrind java $JAVA_OPTS -agentlib:classmapdrawer -jar tiny-rules-sample.jar
