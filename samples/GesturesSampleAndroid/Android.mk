LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := GesturesSample

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_JNI_SHARED_LIBRARIES := libgesturessample libgestures

LOCAL_DEX_PREOPT := false

include $(BUILD_PACKAGE)

include $(LOCAL_PATH)/jni/Android.mk
