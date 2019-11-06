LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    src/ctlr_detector_android.cpp \
    src/ctlr_mgr.cpp \
    src/epoll_mgr.cpp \
    src/epoll_subscriber.cpp \
    src/phys_ctlr.cpp \
    src/virt_ctlr.cpp \
    src/virt_ctlr_combined.cpp \
    src/virt_ctlr_passthrough.cpp \
    src/main.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := libevdev \
    libnl \
    liblog

LOCAL_MODULE := joycond
LOCAL_INIT_RC := android/joycond.rc
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS := -std=c++17 -Wno-error -fexceptions
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_EXECUTABLE)
