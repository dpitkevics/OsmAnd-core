LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifneq ($(OSMAND_BUILDING_NEON_LIBRARY),true)
    LOCAL_MODULE := osmand_icu4c
else
    LOCAL_MODULE := osmand_icu4c_neon
    LOCAL_ARM_NEON := true
endif

LOCAL_EXPORT_CFLAGS := \
    -DU_STATIC_IMPLEMENTATION \
    -DU_LIB_SUFFIX_C_NAME=_osmand \
    -DUCONFIG_NO_FILE_IO=1 \
    -DUCONFIG_NO_SERVICE=1

LOCAL_EXPORT_C_INCLUDES := \
    $(LOCAL_PATH)/upstream.patched/source/common \
    $(LOCAL_PATH)/upstream.patched/source/i18n

LOCAL_CFLAGS := \
    $(LOCAL_EXPORT_CFLAGS) \
    -DU_COMMON_IMPLEMENTATION \
    -DU_I18N_IMPLEMENTATION \
    -DU_HAVE_NL_LANGINFO_CODESET=0

LOCAL_C_INCLUDES := \
    $(LOCAL_EXPORT_C_INCLUDES)

common_SRC_FILES := $(wildcard $(LOCAL_PATH)/upstream.patched/source/common/*.c*)
i18n_SRC_FILES := $(wildcard $(LOCAL_PATH)/upstream.patched/source/i18n/*.c*)

LOCAL_SRC_FILES := \
    $(common_SRC_FILES:$(LOCAL_PATH)/%=%) \
    $(i18n_SRC_FILES:$(LOCAL_PATH)/%=%) \
    upstream.patched/source/stubdata/stubdata.c

include $(BUILD_STATIC_LIBRARY)
