$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test30
LOCAL_PROCESS_SRC := test30.c
$(eval $(call build-test-process))
$(eval $(call build-test-module))
