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
    src/virt_ctlr_pro.cpp \
    src/main.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
    libevdev \
    libnl \
    liblog

LOCAL_REQUIRED_MODULES := \
    Vendor_057e_Product_2006.idc \
    Vendor_057e_Product_2007.idc \
    Vendor_057e_Product_2008.idc \
    Vendor_057e_Product_2008.kl  \
    Vendor_057e_Product_2009_Version_8001.idc \
    Vendor_057e_Product_2009_Version_8001.kl

LOCAL_MODULE := joycond
LOCAL_INIT_RC := android/joycond.rc
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS := -std=c++17 -Wno-error -fexceptions
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := Vendor_057e_Product_2007.idc
LOCAL_SRC_FILES := android/Vendor_057e_Product_2006.idc
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/usr/idc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := Vendor_057e_Product_2006.idc
LOCAL_SRC_FILES := android/Vendor_057e_Product_2006.idc
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/usr/idc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := Vendor_057e_Product_2008.kl
LOCAL_SRC_FILES := android/Vendor_057e_Product_2008.kl
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/usr/keylayout
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := Vendor_057e_Product_2008.idc
LOCAL_SRC_FILES := android/Vendor_057e_Product_2008.idc
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/usr/idc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := Vendor_057e_Product_2009_Version_8001.kl
LOCAL_SRC_FILES := android/Vendor_057e_Product_2008.kl
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/usr/keylayout
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := Vendor_057e_Product_2009_Version_8001.idc
LOCAL_SRC_FILES := android/Vendor_057e_Product_2008.idc
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/usr/idc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := nintendo
include $(BUILD_PREBUILT)
