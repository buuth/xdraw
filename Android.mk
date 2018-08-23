# Copyright (C) 2010 - 17 Degree Company
# Lib: XDraw

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := XDraw
LOCAL_CFLAGS    := 	-D__ANDROID__ \
			-DUNICODE \

LOCAL_C_INCLUDES :=	$(LOCAL_PATH)/../../Android/Native \
			  $(LOCAL_PATH)/../../Source/Core \
			  /cygdrive/E/Android/ndk-r7/sources/cxx-stl/stlport/stlport \
			  $(LOCAL_PATH)/agg_2.5/include \
			  $(LOCAL_PATH)/XDrawRoad/include \

LOCAL_SRC_FILES := AggUtil.cpp \
  Algorithm.cpp \
  MyClip.cpp \
  XBuffer.cpp \
  XDraw.cpp \
  agg_2.5/src/agg_trans_affine.cpp \

include $(BUILD_STATIC_LIBRARY)

