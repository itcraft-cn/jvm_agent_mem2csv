#ifndef _AGENT_COMMON_H
#define _AGENT_COMMON_H

#ifdef J9
#include <ibmjvmti.h>
#define LIB_NAME "j9mem2csv"
#else
#include <jvmti.h>
#define LIB_NAME "hsmem2csv"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#endif
