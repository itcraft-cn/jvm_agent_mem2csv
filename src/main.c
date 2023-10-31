#include <ibmjvmti.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t TAG_PART1 = 0;
volatile sig_atomic_t TAG_PART2 = 0;

#define check_jvmti_error(action_info)                                         \
  if (error != JVMTI_ERROR_NONE) {                                             \
    (*jvmti)->GetErrorName(jvmti, error, (char **)&error_ptr);                 \
    printf("%s, failed: %s\n", action_info, *&error_ptr);                      \
    return JNI_ERR;                                                            \
  }

#define reg_event(jvmti, events, n)                                            \
  for (int i = 0; i < n; i++) {                                                \
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, events[i], \
                                               NULL);                          \
    check_jvmti_error("agent add notification");                               \
  }

#define check_not_null(target, desc)                                           \
  if (target == NULL) {                                                        \
    fatal_error(desc);                                                         \
  }

static int ACTIVE = 0;

/* Send message to stderr or whatever the error output location is and exit  */
void fatal_error(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  (void)vfprintf(stderr, format, ap);
  (void)fflush(stderr);
  va_end(ap);
  exit(3);
}

/* Creates a new jthread */
static jthread alloc_thread(JNIEnv *jni) {
  jclass class_thread = (*jni)->FindClass(jni, "java/lang/Thread");
  check_not_null(class_thread, "Cannot find Thread class\n");
  jmethodID method_thread_new =
      (*jni)->GetMethodID(jni, class_thread, "<init>", "()V");
  check_not_null(method_thread_new, "Cannot find Thread constructor method\n");
  jmethodID method_set_daemon =
      (*jni)->GetMethodID(jni, class_thread, "setDaemon", "(Z)V");
  check_not_null(method_set_daemon, "Cannot find Thread setDaemon method\n");
  jthread thread = (*jni)->NewObject(jni, class_thread, method_thread_new);
  check_not_null(thread, "Cannot create new Thread object\n");
  (*jni)->CallVoidMethod(jni, thread, method_set_daemon, (jboolean)1);
  return thread;
}

void on_iter(jlong class_tag, jlong size, jlong *tag_ptr, jint length,
             void *user_data) {
  int *sum = (int *)user_data;
  (*sum) += size;
}

void output(jvmtiEnv *jvmti, jclass clazz, jlong size) {
  jvmtiError error;
  long error_ptr = 0;
  char *sig;
  char *gsig;
  error = (*jvmti)->GetClassSignature(jvmti, clazz, &sig, &gsig);
  check_jvmti_error("GetClassSignature");
  if (gsig == NULL) {
    printf("Class: %s, size: %d\n", sig, size);
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)sig);
    check_jvmti_error("Deallocate");
  } else {
    printf("Class: %s<%s>, size: %ld\n", sig, gsig, size);
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)sig);
    check_jvmti_error("Deallocate");
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)gsig);
    check_jvmti_error("Deallocate");
  }
}

void walk_heap(jvmtiEnv *jvmti, JNIEnv *jni, jvmtiHeapCallbacks *callbacks) {
  jvmtiError error;
  long error_ptr = 0;
  jint number;
  jclass *classes;
  error = (*jvmti)->GetLoadedClasses(jvmti, &number, &classes);
  check_jvmti_error("GetLoadedClasses");
  for (int i = 0; i < number; i++) {
    jlong class_sum_size = 0;
    error =
        (*jvmti)->IterateThroughHeap(jvmti, JVMTI_HEAP_FILTER_CLASS_TAGGED,
                                     classes[i], callbacks, &class_sum_size);
    check_jvmti_error("IterateThroughHeap");
    if (class_sum_size > 0) {
      output(jvmti, classes[i], class_sum_size);
    }
  }
  error = (*jvmti)->Deallocate(jvmti, (unsigned char *)classes);
  check_jvmti_error("Deallocate");
}

void agent_proc(jvmtiEnv *jvmti, JNIEnv *jni, void *p) {
  sleep(1);
  jvmtiHeapCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.heap_iteration_callback = &on_iter;
  while (ACTIVE) {
    walk_heap(jvmti, jni, &callbacks);
    sleep(1);
  }
  printf("quit thread\n");
}

void on_vm_init(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread) {
  jthread agent_thread = alloc_thread(jni);
  jvmtiError error;
  long error_ptr = 0;
  error = (*jvmti)->RunAgentThread(jvmti, agent_thread, &agent_proc, NULL,
                                   JVMTI_THREAD_MAX_PRIORITY);
  check_jvmti_error("agent start new thread");
  ACTIVE = 1;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options,
                                    void *reserved) {
  jvmtiEnv *jvmti = NULL;
  jvmtiError error;
  long error_ptr = 0;
  /* Get access to JVMTI */
  (*jvm)->GetEnv(jvm, (void **)&jvmti, JVMTI_VERSION_1_0);
  jvmtiCapabilities capabilities;
  memset(&capabilities, 0, sizeof(capabilities));
  capabilities.can_tag_objects = 1;
  error = (*jvmti)->AddCapabilities(jvmti, &capabilities);
  check_jvmti_error("agent add capability");
  jvmtiEventCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.VMInit = &on_vm_init;
  error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));
  check_jvmti_error("agent add callback");
  jvmtiEvent events[] = {JVMTI_EVENT_VM_INIT};
  reg_event(jvmti, events, 1);
  printf("agent is ready\n");
  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) { printf("agent is off\n"); }

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options,
                                      void *reserved) {
  ACTIVE = 0;
  sleep(3);
  return JNI_OK;
}
