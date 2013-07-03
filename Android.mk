# Copyright (C) 2013 Hisense Co.,LTD
# Author wangbaoyun@hisense.com

LOCAL_PATH:=		$(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= input
LOCAL_SRC_FILES:= input.c
LOCAL_SYSTEM_SHARED_LIBRARIES:= libc

include $(BUILD_EXECUTABLE)
