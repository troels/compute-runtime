#
# Copyright (C) 2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

target_sources(${TARGET_NAME} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/debug_session_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/eu_thread_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/mock_debug_session.h
               ${CMAKE_CURRENT_SOURCE_DIR}${BRANCH_DIR_SUFFIX}test_debug_api.cpp
)

add_subdirectories()
