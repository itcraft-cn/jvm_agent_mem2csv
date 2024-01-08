#include "agent_args.h"
#include "agent_common.h"

#ifndef _AGENT_TRACE_H
#define _AGENT_TRACE_H

#define check_jvmti_error_ret_int(action_info)                                 \
  if (error != JVMTI_ERROR_NONE) {                                             \
    (*jvmti)->GetErrorName(jvmti, error, &error_ptr);                          \
    printf("%s, failed: %s\n", action_info, error_ptr);                        \
    return JNI_ERR;                                                            \
  }

#define check_jvmti_error_ret(action_info)                                     \
  if (error != JVMTI_ERROR_NONE) {                                             \
    (*jvmti)->GetErrorName(jvmti, error, &error_ptr);                          \
    printf("%s, failed: %s\n", action_info, error_ptr);                        \
    return;                                                                    \
  }

#define reg_event(jvmti, events, n)                                            \
  for (int i = 0; i < n; i++) {                                                \
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, events[i], \
                                               NULL);                          \
    check_jvmti_error_ret_int("agent add notification");                       \
  }

#define check_not_null(target, desc)                                           \
  if (target == NULL) {                                                        \
    fatal_error(desc);                                                         \
  }

jint agent_load(JavaVM *jvm, char *options, void *reserved, pid_t *pid,
                AgentCfg *cfg);
void agent_unload(JavaVM *vm);

#endif
