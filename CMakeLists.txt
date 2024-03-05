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

cmake_minimum_required(VERSION 3.20)

project(tean LANGUAGES C CXX)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE TEAN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE TEAN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(tean STATIC ${TEAN_HEADERS} ${TEAN_SOURCES})
add_library(tean::tean ALIAS tean)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/tean/" PREFIX "include" FILES ${TEAN_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/" PREFIX "src" FILES ${TEAN_SOURCES})
target_include_directories(tean PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

get_filename_component(TEAN_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH)
get_filename_component(MAIN_PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR} REALPATH)
if(TEAN_PROJECT_SOURCE_DIR STREQUAL MAIN_PROJECT_SOURCE_DIR)
   if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
      target_compile_options(tean PUBLIC /W4)
   else()
      target_compile_options(tean PUBLIC -Wall -Wextra -Wpedantic)
   endif()
   enable_testing()
   add_subdirectory(tests)
   set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT tests)
endif()
