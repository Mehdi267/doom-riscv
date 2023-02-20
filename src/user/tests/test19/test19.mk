$(eval $(call clear-module-vars))
LOCAL_MODULE_PATH := $(call my-dir)

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := test19
LOCAL_PROCESS_SRC := test19-crl.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := cons_rd0
LOCAL_PROCESS_SRC := cons_rd0-crl.c
$(eval $(call build-test-process))

$(eval $(call clear-process-vars))
LOCAL_PROCESS_NAME := cons_rdN
LOCAL_PROCESS_SRC := cons_rdN-crl.c
$(eval $(call build-test-process))


$(eval $(call build-test-module))
