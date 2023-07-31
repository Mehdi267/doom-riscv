$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test27
LOCAL_PROCESS_SRC := test27.c
$(eval $(call build-test-process))
$(eval $(call build-test-module))
