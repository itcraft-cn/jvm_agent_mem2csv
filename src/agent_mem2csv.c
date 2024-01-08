#include "agent_mem2csv.h"

const int BUF_SIZE = 65536;
static char BUF[65536];

static long CSV_FILE_WALKER = 1;
const int MAX_FILE_NAME_LEN = 256;

static int ACTIVE = 0;

static pid_t *JVM_PID;

static AgentCfg *CFG;

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

int on_iter(jlong class_tag, jlong size, jlong *tag_ptr, jint length,
            void *user_data) {
  int *sum = (int *)user_data;
  (*sum) += size;
  return JVMTI_VISIT_OBJECTS;
}

void output(int offset, jvmtiEnv *jvmti, jclass clazz, jlong size) {
  jvmtiError error;
  char *error_ptr;
  char *sig;
  char *gsig;
  error = (*jvmti)->GetClassSignature(jvmti, clazz, &sig, &gsig);
  check_jvmti_error_ret("GetClassSignature");
  if (gsig == NULL) {
    sprintf(BUF + offset, "\"%s\",%lld\n", sig, size);
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)sig);
    check_jvmti_error_ret("Deallocate");
  } else {
    sprintf(BUF + offset, "\"%s<%s>\",%lld\n", sig, gsig, size);
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)sig);
    check_jvmti_error_ret("Deallocate");
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)gsig);
    check_jvmti_error_ret("Deallocate");
  }
}

void reset_buf(int *offset) {
  memset(BUF, 0, BUF_SIZE);
  *offset = 0;
}

void walk_heap(jvmtiEnv *jvmti, JNIEnv *jni, jvmtiHeapCallbacks *callbacks) {
  jvmtiError error;
  char *error_ptr;
  jint number;
  jclass *classes;
  error = (*jvmti)->GetLoadedClasses(jvmti, &number, &classes);
  check_jvmti_error_ret("GetLoadedClasses");
  int offset = 0;
  reset_buf(&offset);
  char filename[MAX_FILE_NAME_LEN];
  sprintf(filename, "%s/jvm_objects_%d_%06ld.csv", CFG->output_dir, *JVM_PID,
          CSV_FILE_WALKER++);
  FILE *fd = fopen(filename, "w+");
  for (int i = 0; i < number; i++) {
    jlong class_sum_size = 0;
    error =
        (*jvmti)->IterateThroughHeap(jvmti, JVMTI_HEAP_FILTER_CLASS_TAGGED,
                                     classes[i], callbacks, &class_sum_size);
    check_jvmti_error_ret("IterateThroughHeap");
    if (class_sum_size > 0) {
      output(offset, jvmti, classes[i], class_sum_size);
      offset = strlen(BUF);
      if (offset + 1024 > BUF_SIZE) {
        fwrite(BUF, offset, 1, fd);
        reset_buf(&offset);
      }
    }
  }
  if (offset > 0) {
    fwrite(BUF, offset, 1, fd);
  }
  fclose(fd);
  error = (*jvmti)->Deallocate(jvmti, (unsigned char *)classes);
  check_jvmti_error_ret("Deallocate");
}

void agent_proc(jvmtiEnv *jvmti, JNIEnv *jni, void *p) {
  sleep(CFG->start_idle_time);
  jvmtiHeapCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.heap_iteration_callback = &on_iter;
  while (ACTIVE) {
    walk_heap(jvmti, jni, &callbacks);
    sleep(CFG->scan_interval);
  }
  printf("quit thread\n");
}

void on_vm_init(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread) {
  jthread agent_thread = alloc_thread(jni);
  jvmtiError error;
  char *error_ptr;
  error = (*jvmti)->RunAgentThread(jvmti, agent_thread, &agent_proc, NULL,
                                   JVMTI_THREAD_MAX_PRIORITY);
  check_jvmti_error_ret("agent start new thread");
  ACTIVE = 1;
}

jint agent_load(JavaVM *jvm, char *options, void *reserved, pid_t *pid,
                AgentCfg *cfg) {
  JVM_PID = pid;
  CFG = cfg;
  jvmtiError error;
  char *error_ptr;
  jvmtiEnv *jvmti = NULL;
  (*jvm)->GetEnv(jvm, (void **)&jvmti, JVMTI_VERSION_1_0);
  jvmtiCapabilities capabilities;
  memset(&capabilities, 0, sizeof(capabilities));
  capabilities.can_tag_objects = 1;
  error = (*jvmti)->AddCapabilities(jvmti, &capabilities);
  check_jvmti_error_ret_int("agent add capability");
  jvmtiEventCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.VMInit = &on_vm_init;
  error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));
  check_jvmti_error_ret_int("agent add callback");
  jvmtiEvent events[] = {JVMTI_EVENT_VM_INIT};
  reg_event(jvmti, events, 1);
  printf("agent is ready\n");
  return JNI_OK;
}

void agent_unload(JavaVM *vm) {
  ACTIVE = 0;
  sleep(3);
  if (CFG->setup_output_dir == 1) {
    free(CFG->output_dir);
  }
  printf("agent is off\n");
}
