#!/bin/bash

CURR_DIR=$(pwd)
DEBUG_LIB_PATH=$CURR_DIR/build/linux/x86_64/debug
RELEASE_LIB_PATH=$CURR_DIR/build/linux/x86_64/release

cd jar_test || exit 1

export JAVA_HOME="$JAVA_HOME8_IBM"
export PATH="$JAVA_HOME/bin":"$PATH"
export JAVA_OPTS="-Xmx2G -Xms2G"

OLD_LD_PATH=$LD_LIBRARY_PATH

echo normal call
time java $JAVA_OPTS -jar tiny-rules-sample.jar

echo call with agent, debug
time LD_LIBRARY_PATH=$DEBUG_LIB_PATH:$OLD_LD_PATH java $JAVA_OPTS -agentlib:j9mem2csv=3 -jar tiny-rules-sample.jar

echo call with agent, debug
time LD_LIBRARY_PATH=$RELEASE_LIB_PATH:$OLD_LD_PATH java $JAVA_OPTS -agentlib:j9mem2csv=3 -jar tiny-rules-sample.jar

#time valgrind java $JAVA_OPTS -agentlib:j9mem2csv -jar tiny-rules-sample.jar
