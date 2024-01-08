#include "agent_common.h"

#ifndef _AGENT_ARGS_H
#define _AGENT_ARGS_H

typedef struct {
  int scan_interval;
  int start_idle_time;
  int setup_output_dir;
  char *output_dir;
} AgentCfg;

AgentCfg *parse_options(char *options);

#endif
