#include <ibmjvmti.h>

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
  return 0;
}
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {}
JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options,
                                      void *reserved) {
  return 0;
}
