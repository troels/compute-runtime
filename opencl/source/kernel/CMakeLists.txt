#
# Copyright (C) 2018-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(RUNTIME_SRCS_KERNEL
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}${BRANCH_DIR_SUFFIX}get_additional_kernel_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_transformer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_transformer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_info_cl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel_objects_for_aux_translation.h
    ${CMAKE_CURRENT_SOURCE_DIR}${BRANCH_DIR_SUFFIX}kernel_extra.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/multi_device_kernel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/multi_device_kernel.h
)
target_sources(${NEO_STATIC_LIB_NAME} PRIVATE ${RUNTIME_SRCS_KERNEL})
set_property(GLOBAL PROPERTY RUNTIME_SRCS_KERNEL ${RUNTIME_SRCS_KERNEL})
