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

include(FetchContent)

FetchContent_Declare(
   tilib
   # Download Step Options
   GIT_PROGRESS ON
   GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
   GIT_REPOSITORY https://github.com/TulipCharts/tulipindicators.git
   GIT_SHALLOW ON
   GIT_SUBMODULES_RECURSE ON
   GIT_TAG master
)
FetchContent_Populate(tilib)
file(GLOB TULIPINDICATORS_HEADERS "${tilib_SOURCE_DIR}/indicators/*.h" "${tilib_SOURCE_DIR}/indicators.h")
file(GLOB TULIPINDICATORS_SOURCES "${tilib_SOURCE_DIR}/indicators/*.c" "${tilib_SOURCE_DIR}/indicators.c")
add_library(tulipindicators STATIC ${TULIPINDICATORS_HEADERS} ${TULIPINDICATORS_SOURCES})
set_target_properties(
   tulipindicators
   PROPERTIES
      FOLDER thirdparty
      LIBRARY_OUTPUT_DIRECTORY "${tilib_BINARY_DIR}"
)
target_include_directories(tulipindicators PUBLIC "${tilib_SOURCE_DIR}")