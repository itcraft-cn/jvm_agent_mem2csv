#include "agent_args.h"
#include "agent_common.h"
#include "agent_mem2csv.h"

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options,
                                    void *reserved) {
  int pid = getpid();
  printf("agent will attach the jvm[pid: %d]\n", pid);
  return agent_load(jvm, options, reserved, &pid, parse_options(options));
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
  agent_unload(vm);
}

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options,
                                      void *reserved) {
  return JNI_OK;
}
