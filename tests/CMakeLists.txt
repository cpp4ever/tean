#[[
   Part of the TeAn Project (https://github.com/cpp4ever/tean), under the MIT License
   SPDX-License-Identifier: MIT

   Copyright (c) 2024 Mikhail Smirnov

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
]]

include("${TEAN_PROJECT_SOURCE_DIR}/thirdparty/CMakeSanitizer.cmake")
include("${TEAN_PROJECT_SOURCE_DIR}/thirdparty/googletest.cmake")
include("${TEAN_PROJECT_SOURCE_DIR}/thirdparty/ta-lib.cmake")
include("${TEAN_PROJECT_SOURCE_DIR}/thirdparty/tulipindicators.cmake")

file(GLOB_RECURSE TEAN_TESTS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE TEAN_TESTS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
set(
   TEAN_TESTS_THIRDPARTY
   "${TEAN_PROJECT_SOURCE_DIR}/thirdparty/CMakeSanitizer.cmake"
   "${TEAN_PROJECT_SOURCE_DIR}/thirdparty/googletest.cmake"
   "${TEAN_PROJECT_SOURCE_DIR}/thirdparty/ta-lib.cmake"
   "${TEAN_PROJECT_SOURCE_DIR}/thirdparty/tulipindicators.cmake"
)
add_executable(tests ${TEAN_TESTS_HEADERS} ${TEAN_TESTS_SOURCES} ${TEAN_TESTS_THIRDPARTY})
add_executable(tean::tests ALIAS tests)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/" FILES ${TEAN_TESTS_HEADERS} ${TEAN_TESTS_SOURCES})
source_group(TREE "${TEAN_PROJECT_SOURCE_DIR}/" FILES ${TEAN_TESTS_THIRDPARTY})
target_include_directories(tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(tests PRIVATE gmock_main ta_lib_static tean tulipindicators)
available_cxx_sanitizers(CXX_SANITIZERS)
list(LENGTH CXX_SANITIZERS CXX_SANITIZERS_LENGTH)
if(CXX_SANITIZERS_LENGTH GREATER 0)
   list(GET CXX_SANITIZERS 0 DEFAULT_CXX_SANITIZER)
   set(SANITIZER ${DEFAULT_CXX_SANITIZER} CACHE STRING "Choose sanitizer for the build")
   set_property(CACHE SANITIZER PROPERTY STRINGS ${CXX_SANITIZERS})
   if(SANITIZER STREQUAL "UBSan")
      target_cxx_ubsan(
         tests
         FLAGS
            print_suppressions=true
            print_stacktrace=true
            report_error_type=true
            silence_unsigned_overflow=true
      )
   else()
      target_cxx_sanitizer(tests ${SANITIZER})
   endif()
endif()
get_filename_component(CXX_LINKER_DIRECTORY ${CMAKE_LINKER} DIRECTORY)
string(LENGTH "${CXX_LINKER_DIRECTORY}" CXX_LINKER_DIRECTORY_LENGTH)
if(CXX_LINKER_DIRECTORY_LENGTH GREATER 0)
   set(TEAN_CTEST_PROPERTIES ENVIRONMENT "PATH=${CXX_LINKER_DIRECTORY};$ENV{PATH}")
endif()

include(GoogleTest)
gtest_discover_tests(tests PROPERTIES ${TEAN_CTEST_PROPERTIES})
