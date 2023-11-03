#ifdef J9
#include <ibmjvmti.h>
#else
#include <jvmti.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define check_jvmti_error(action_info)                                         \
  if (error != JVMTI_ERROR_NONE) {                                             \
    (*jvmti)->GetErrorName(jvmti, error, error_ptr);                           \
    printf("%s, failed: %s\n", action_info, error_ptr);                        \
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

static pid_t JVM_PID;

const int BUF_SIZE = 65536;
static char BUF[65536];

static long CSV_FILE_WALKER = 1;
const int MAX_FILE_NAME_LEN = 256;

static int ACTIVE = 0;

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

typedef struct {
  int scan_interval;
  int start_idle_time;
  int setup_output_dir;
  char *output_dir;
} AgentCfg;

static AgentCfg cfg;

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

void output(int offset, jvmtiEnv *jvmti, jclass clazz, jlong size) {
  jvmtiError error;
  char *error_ptr;
  char *sig;
  char *gsig;
  error = (*jvmti)->GetClassSignature(jvmti, clazz, &sig, &gsig);
  check_jvmti_error("GetClassSignature");
  if (gsig == NULL) {
    sprintf(BUF + offset, "\"%s\",%ld\n", sig, size);
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)sig);
    check_jvmti_error("Deallocate");
  } else {
    sprintf(BUF + offset, "\"%s<%s>\",%ld\n", sig, gsig, size);
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)sig);
    check_jvmti_error("Deallocate");
    error = (*jvmti)->Deallocate(jvmti, (unsigned char *)gsig);
    check_jvmti_error("Deallocate");
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
  check_jvmti_error("GetLoadedClasses");
  int offset = 0;
  reset_buf(&offset);
  char filename[MAX_FILE_NAME_LEN];
  sprintf(filename, "%s/jvm_objects_%ld_%06d.csv", cfg.output_dir, JVM_PID,
          CSV_FILE_WALKER++);
  FILE *fd = fopen(filename, "w+");
  for (int i = 0; i < number; i++) {
    jlong class_sum_size = 0;
    error =
        (*jvmti)->IterateThroughHeap(jvmti, JVMTI_HEAP_FILTER_CLASS_TAGGED,
                                     classes[i], callbacks, &class_sum_size);
    check_jvmti_error("IterateThroughHeap");
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
  check_jvmti_error("Deallocate");
}

void agent_proc(jvmtiEnv *jvmti, JNIEnv *jni, void *p) {
  sleep(cfg.start_idle_time);
  jvmtiHeapCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.heap_iteration_callback = &on_iter;
  while (ACTIVE) {
    walk_heap(jvmti, jni, &callbacks);
    sleep(cfg.scan_interval);
  }
  printf("quit thread\n");
}

void on_vm_init(jvmtiEnv *jvmti, JNIEnv *jni, jthread thread) {
  jthread agent_thread = alloc_thread(jni);
  jvmtiError error;
  char *error_ptr;
  error = (*jvmti)->RunAgentThread(jvmti, agent_thread, &agent_proc, NULL,
                                   JVMTI_THREAD_MAX_PRIORITY);
  check_jvmti_error("agent start new thread");
  ACTIVE = 1;
}

void print_help() {
#ifdef J9
  printf("usage: java -agentlib:j9mem2csv=<options>\n");
#else
  printf("usage:\t\tjava -agentlib:hsmem2csv=<options>\n");
#endif
  printf("options:\n");
  printf("\t\thelp\t\t"
         "print this message\n");
  printf("\t\tinterval\t"
         "the interval between two scan actions\n");
  printf("\t\tidle\t\t"
         "the idle time before the first scan\n");
  printf("\t\toutput\t\t"
         "the directory for the output csv files\n");
  printf("example:\tjava -agentlib:hsmem2csv=help\n");
  printf("example:\tjava "
         "-agentlib:hsmem2csv=interval=600,idle=1200,output=/tmp\n");
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

void parse_options(char *options) {
  memset(&cfg, 0, sizeof(AgentCfg));
  cfg.scan_interval = DEFAULT_SCAN_INTERVAL;
  cfg.start_idle_time = DEFAULT_IDLE_TIME;
  cfg.output_dir = DEFAULT_OUTPUT_DIR;
  if (options != NULL) {
    int len = strlen(options);
    if (len > 0) {
      parse(options, len);
    }
  }
  printf("will use cfg: scan_interval=%d, start_idle_time=%d, output_dir=%s\n",
         cfg.scan_interval, cfg.start_idle_time, cfg.output_dir);
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options,
                                    void *reserved) {
  JVM_PID = getpid();
  printf("agent will attach the jvm[pid: %d]\n", JVM_PID);
  parse_options(options);
  jvmtiError error;
  char *error_ptr;
  jvmtiEnv *jvmti = NULL;
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

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
  ACTIVE = 0;
  sleep(3);
  if (cfg.setup_output_dir == 1) {
    free(cfg.output_dir);
  }
  printf("agent is off\n");
}

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options,
                                      void *reserved) {
  return JNI_OK;
}
