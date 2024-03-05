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

include(${TEAN_PROJECT_SOURCE_DIR}/thirdparty/googletest.cmake)
include(${TEAN_PROJECT_SOURCE_DIR}/thirdparty/ta-lib.cmake)
include(${TEAN_PROJECT_SOURCE_DIR}/thirdparty/tulipindicators.cmake)

function(set_target_sanitizer TARGET SANITIZER)
   string(TOLOWER "${SANITIZER}" sanitizer)
   if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
      if(sanitizer STREQUAL "address")
         set_target_properties(
            ${TARGET}
            PROPERTIES
               MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase
               MSVC_RUNTIME_LIBRARY MultiThreadedDLL
         )
         target_compile_options(${TARGET} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/fsanitize=address /Oy- /Zi>)
         target_link_options(${TARGET} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:/INCREMENTAL:NO>)
      else()
         message(FATAL_ERROR "Unknown ${CMAKE_CXX_COMPILER_ID} sanitizer: ${SANITIZER}")
      endif()
   elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$")
      if(sanitizer STREQUAL "address")
         target_compile_options(
            ${TARGET}
            PRIVATE
               $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address -fsanitize-address-use-after-return=always>
               $<$<CXX_COMPILER_ID:Clang>:-fsanitize=leak>
         )
         target_link_options(
            ${TARGET}
            PRIVATE
               $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address -fsanitize-address-use-after-return=always>
               $<$<CXX_COMPILER_ID:Clang>:-fsanitize=leak>
         )
      elseif(sanitizer STREQUAL "ub")
         target_compile_options(
            ${TARGET}
            PRIVATE
               $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fsanitize=float-divide-by-zero -fsanitize=integer -fsanitize=undefined>
         )
         target_link_options(
            ${TARGET}
            PRIVATE
               $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fsanitize=float-divide-by-zero -fsanitize=integer -fsanitize=undefined>
         )
      else()
         message(FATAL_ERROR "Unknown ${CMAKE_CXX_COMPILER_ID} sanitizer: ${SANITIZER}")
      endif()
   elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      if(sanitizer STREQUAL "address")
         target_compile_options(${TARGET} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fsanitize=address -fsanitize=leak>)
         target_link_options(${TARGET} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fsanitize=address -fsanitize=leak>)
      elseif(sanitizer STREQUAL "ub")
         target_compile_options(${TARGET} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fsanitize=undefined>)
         target_link_options(${TARGET} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-fno-omit-frame-pointer -fsanitize=undefined>)
      else()
         message(FATAL_ERROR "Unknown ${CMAKE_CXX_COMPILER_ID} sanitizer: ${SANITIZER}")
      endif()
   endif()
endfunction()

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
   if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      set(SANITIZER "Address" CACHE STRING "Choose sanitizer for the build")
      set_property(CACHE SANITIZER PROPERTY STRINGS "Address;")
   endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
   set(SANITIZER "Address" CACHE STRING "Choose sanitizer for the build")
   set_property(CACHE SANITIZER PROPERTY STRINGS "Address;UB;")
endif()

file(GLOB_RECURSE TEAN_TESTS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE TEAN_TESTS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
set(
   TEAN_TESTS_THIRDPARTY
   "${TEAN_PROJECT_SOURCE_DIR}/thirdparty/googletest.cmake"
   "${TEAN_PROJECT_SOURCE_DIR}/thirdparty/ta-lib.cmake"
   "${TEAN_PROJECT_SOURCE_DIR}/thirdparty/tulipindicators.cmake"
)
add_executable(tests ${TEAN_TESTS_HEADERS} ${TEAN_TESTS_SOURCES} ${TEAN_TESTS_THIRDPARTY})
add_executable(tean::tests ALIAS tests)
if(NOT "${SANITIZER}" STREQUAL "")
   set_target_sanitizer(gmock_main ${SANITIZER})
   set_target_sanitizer(ta_lib_static ${SANITIZER})
   set_target_sanitizer(tean ${SANITIZER})
   set_target_sanitizer(tests ${SANITIZER})
   set_target_sanitizer(tulipindicators ${SANITIZER})
endif()
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/" FILES ${TEAN_TESTS_HEADERS} ${TEAN_TESTS_SOURCES})
source_group(TREE "${TEAN_PROJECT_SOURCE_DIR}/" FILES ${TEAN_TESTS_THIRDPARTY})
target_include_directories(tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(tests PRIVATE gmock_main ta_lib_static tean tulipindicators)

include(GoogleTest)
gtest_discover_tests(tests PROPERTIES ${TEAN_CTEST_PROPERTIES})
