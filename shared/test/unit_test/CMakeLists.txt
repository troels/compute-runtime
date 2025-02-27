#
# Copyright (C) 2019-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(SHARED_TEST_PROJECTS_FOLDER "neo shared")
if(NOT SKIP_UNIT_TESTS)

  add_custom_target(unit_tests)

  set(TARGET_NAME neo_shared_tests)

  include(${NEO_SOURCE_DIR}/cmake/setup_ult_global_flags.cmake)

  function(ADD_SUPPORTED_TEST_PRODUCT_FAMILIES_DEFINITION)
    set(NEO_SUPPORTED_PRODUCT_FAMILIES ${ALL_TESTED_PRODUCT_FAMILY})
    string(REPLACE ";" "," NEO_SUPPORTED_PRODUCT_FAMILIES "${NEO_SUPPORTED_PRODUCT_FAMILIES}")
    add_definitions(-DSUPPORTED_TEST_PRODUCT_FAMILIES=${NEO_SUPPORTED_PRODUCT_FAMILIES})
  endfunction()

  ADD_SUPPORTED_TEST_PRODUCT_FAMILIES_DEFINITION()
  link_libraries(${ASAN_LIBS} ${TSAN_LIBS})

  add_executable(${TARGET_NAME}
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/test_mode.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/tests_configuration.h
                 ${NEO_SOURCE_DIR}/opencl/source/compiler_interface/default_cache_config.cpp
                 ${NEO_SOURCE_DIR}/opencl/test/unit_test/helpers/kernel_binary_helper_hash_value.cpp
                 ${NEO_SOURCE_DIR}/opencl/test/unit_test/libult/io_functions.cpp
                 ${NEO_SOURCE_DIR}/opencl/test/unit_test/test_macros/test_checks_ocl.cpp
                 ${NEO_SHARED_DIRECTORY}/helpers/allow_deferred_deleter.cpp
                 ${NEO_SHARED_TEST_DIRECTORY}/common/helpers/api_specific_config_shared_tests.cpp
                 ${NEO_SHARED_TEST_DIRECTORY}/common/test_macros/test_checks_shared.cpp
                 ${NEO_SHARED_TEST_DIRECTORY}/common/test_configuration/unit_tests/ult_configuration.cpp
                 $<TARGET_OBJECTS:mock_gmm>
                 $<TARGET_OBJECTS:igdrcl_libult_common>
                 $<TARGET_OBJECTS:neo_libult_cs>
  )

  if(UNIX)
    target_sources(${TARGET_NAME} PRIVATE
                   ${NEO_SOURCE_DIR}/opencl/source/dll/linux/os_interface.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/linux/create_drm_memory_manager.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/linux/drm_neo_create.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/linux/options.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/linux/sys_calls_linux_ult.cpp
                   ${NEO_SHARED_TEST_DIRECTORY}/common/mocks/linux/mock_drm_memory_manager.cpp
                   ${NEO_SHARED_TEST_DIRECTORY}/common/mocks/linux/mock_drm_memory_manager.h
    )
    if(NOT DISABLE_WDDM_LINUX)
      target_sources(${TARGET_NAME} PRIVATE
                     ${NEO_SHARED_DIRECTORY}/gmm_helper/windows/gmm_memory.cpp
                     ${NEO_SHARED_DIRECTORY}/os_interface/windows/wddm/wddm_create.cpp
      )
    endif()
  else()
    target_sources(${TARGET_NAME} PRIVATE
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/create_wddm_memory_manager.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/options.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/os_memory_virtual_alloc_ult.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/sys_calls.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/ult_dxcore_factory.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/ult_dxgi_factory.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/wddm_calls.cpp
                   ${NEO_SOURCE_DIR}/opencl/test/unit_test/os_interface/windows/wddm_create.cpp
    )
  endif()

  set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${ASAN_FLAGS})
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "${SHARED_TEST_PROJECTS_FOLDER}")

  target_include_directories(${TARGET_NAME} PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}
                             ${ENGINE_NODE_DIR}
                             ${NEO_SHARED_TEST_DIRECTORY}/common/test_macros/header${BRANCH_DIR_SUFFIX}
                             ${NEO_SHARED_TEST_DIRECTORY}/common/helpers/includes${BRANCH_DIR_SUFFIX}
  )

  if(UNIX)
    target_link_libraries(${TARGET_NAME} pthread rt)
  else()
    target_link_libraries(${TARGET_NAME} dbghelp)
  endif()

  target_link_libraries(${TARGET_NAME}
                        gmock-gtest
                        ${NEO_STATICALLY_LINKED_LIBRARIES_MOCKABLE}
                        compute_runtime_mockable_extra
  )

  if(MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES
                          VS_DEBUGGER_COMMAND_ARGUMENTS "--gtest_filter=* --gtest_catch_exceptions=0 --enable_default_listener --disable_pagefaulting_tests"
                          VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
    )
  endif()

  add_subdirectory(${NEO_SHARED_TEST_DIRECTORY}/common "${NEO_BUILD_DIR}/shared/test/common")

  if(NOT SKIP_SHARED_UNIT_TESTS)
    add_subdirectories()
  endif()

  get_property(NEO_CORE_tests_mocks GLOBAL PROPERTY NEO_CORE_tests_mocks)
  target_sources(${TARGET_NAME} PRIVATE ${NEO_CORE_tests_mocks})
  add_dependencies(${TARGET_NAME} prepare_test_kernel_for_shared)
  add_dependencies(unit_tests ${TARGET_NAME})

  create_project_source_tree(${TARGET_NAME})
endif()
