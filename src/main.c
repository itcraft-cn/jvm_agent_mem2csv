#include <ibmjvmti.h>
#include <stdio.h>
#include <stdlib.h>

void on_obj_alloc(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread,
                  jobject object, jclass object_klass, jlong size) {
  jvmtiThreadInfo thread_info;
  memset(&thread_info, 0, sizeof(thread_info));
  (*jvmti_env)->GetThreadInfo(jvmti_env, thread, &thread_info);
  long ptr1 = 0;
  long ptr2 = 0;
  (*jvmti_env)
      ->GetClassSignature(jvmti_env, object_klass, (char **)&ptr1,
                          (char **)&ptr2);
  printf("%s, %s, %s, %ld\n", thread_info.name, *&ptr1, *&ptr2, size);
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
  capabilities.can_generate_vm_object_alloc_events = 1;
  error = (*jvmti)->AddCapabilities(jvmti, &capabilities);
  if (error == JVMTI_ERROR_NONE) {
    printf("agent add capability success\n");
  } else {
    (*jvmti)->GetErrorName(jvmti, error, (char **)&error_ptr);
    printf("Error: %s\n", *&error_ptr);
    return JNI_ERR;
  }
  jvmtiEventCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.VMObjectAlloc = &on_obj_alloc;
  error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));
  if (error == JVMTI_ERROR_NONE) {
    printf("agent add callback success\n");
  } else {
    (*jvmti)->GetErrorName(jvmti, error, (char **)&error_ptr);
    printf("Error: %s\n", *&error_ptr);
    return JNI_ERR;
  }
  error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                                             JVMTI_EVENT_VM_OBJECT_ALLOC, NULL);
  if (error == JVMTI_ERROR_NONE) {
    printf("agent add callback success\n");
  } else {
    (*jvmti)->GetErrorName(jvmti, error, (char **)&error_ptr);
    printf("Error: %s\n", *&error_ptr);
    return JNI_ERR;
  }
  printf("agent is ready\n");
  return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) { printf("agent is off\n"); }

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *vm, char *options,
                                      void *reserved) {
  return JNI_OK;
}
