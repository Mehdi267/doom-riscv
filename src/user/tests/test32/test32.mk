$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test32
LOCAL_PROCESS_SRC := test32.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test_execve
LOCAL_PROCESS_SRC := test_execve.c
$(eval $(call build-test-process))

$(eval $(call build-test-module))
