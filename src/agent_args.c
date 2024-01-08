#include "agent_args.h"

const char *OPTION_HELP = "help";
const char *OPTION_SCAN_INTERVAL = "interval=";
const char *OPTION_START_IDLE_TIME = "idle=";
const char *OPTION_OUTPUT_DIR = "output=";

const int OPTION_HELP_LEN = 4;
const int OPTION_SCAN_INTERVAL_LEN = 9;
const int OPTION_START_IDLE_TIME_LEN = 5;
const int OPTION_OUTPUT_DIR_LEN = 7;

const int DEFAULT_SCAN_INTERVAL = 10 * 60;
const int DEFAULT_IDLE_TIME = 20 * 60;
const char *DEFAULT_OUTPUT_DIR = "/tmp";

static AgentCfg cfg;

void print_help() {
  printf("usage: java -agentlib:%s=<options>\n", LIB_NAME);
  printf("options:\n");
  printf("\t\thelp\t\t"
         "print this message\n");
  printf("\t\tinterval\t"
         "the interval between two scan actions\n");
  printf("\t\tidle\t\t"
         "the idle time before the first scan\n");
  printf("\t\toutput\t\t"
         "the directory for the output csv files\n");
  printf("example:\tjava -agentlib:%s=help\n", LIB_NAME);
  printf("example:\tjava "
         "-agentlib:%s=interval=600,idle=1200,output=/tmp\n",
         LIB_NAME);
}

int parse_int(const char *ptr, int len) {
  char *p = (char *)malloc(len);
  memcpy(p, ptr, len);
  int val = atoi(p);
  free(p);
  return val;
}

char *parse_str(const char *ptr, int len) {
  char *p = (char *)malloc(len);
  memcpy(p, ptr, len);
  return p;
}

void parse(const char *options, int len) {
  int offset = 0;
  int last_idx = len - 1;
  for (int i = 0; i < len; i++) {
    printf("%i\n", options[i]);
    if (options[i] == ',' || i == last_idx) {
      if (strncmp(OPTION_HELP, options + offset, OPTION_HELP_LEN) == 0) {
        print_help();
        exit(1);
      } else if (strncmp(OPTION_SCAN_INTERVAL, options + offset,
                         OPTION_SCAN_INTERVAL_LEN) == 0) {
        cfg.scan_interval =
            parse_int(options + offset + OPTION_SCAN_INTERVAL_LEN, i - offset);
        offset = i + 1;
      } else if (strncmp(OPTION_START_IDLE_TIME, options + offset,
                         OPTION_START_IDLE_TIME_LEN) == 0) {
        cfg.start_idle_time = parse_int(
            options + offset + OPTION_START_IDLE_TIME_LEN, i - offset);
        offset = i + 1;
      } else if (strncmp(OPTION_OUTPUT_DIR, options + offset,
                         OPTION_OUTPUT_DIR_LEN) == 0) {
        cfg.setup_output_dir = 1;
        cfg.output_dir =
            parse_str(options + offset + OPTION_OUTPUT_DIR_LEN, i - offset);
        offset = i + 1;
      }
    }
  }
}

AgentCfg *parse_options(char *options) {
  memset(&cfg, 0, sizeof(AgentCfg));
  cfg.scan_interval = DEFAULT_SCAN_INTERVAL;
  cfg.start_idle_time = DEFAULT_IDLE_TIME;
  cfg.output_dir = (char *)DEFAULT_OUTPUT_DIR;
  if (options != NULL) {
    int len = strlen(options);
    if (len > 0) {
      parse(options, len);
    }
  }
  printf("will use cfg: scan_interval=%d, start_idle_time=%d, output_dir=%s\n",
         cfg.scan_interval, cfg.start_idle_time, cfg.output_dir);
  return &cfg;
}
