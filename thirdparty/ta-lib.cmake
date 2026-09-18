#[[
   Part of the TeAn Project (https://github.com/cpp4ever/tean), under the MIT License
   SPDX-License-Identifier: MIT

   Copyright (c) 2024-2026 Mikhail Smirnov

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

include(CMakeThirdpartyTargets)
include(FetchContent)

set(BUILD_DEV_TOOLS OFF CACHE BOOL "Skip dev tools" FORCE)
if(WIN32 AND NOT DEFINED ENV{Platform})
   string(TOLOWER ${CMAKE_VS_PLATFORM_NAME_DEFAULT} TEAN_VS_PLATFORM_NAME)
   set(ENV{Platform} ${TEAN_VS_PLATFORM_NAME})
endif()
FetchContent_Declare(
   talib
   EXCLUDE_FROM_ALL
   SYSTEM
   # Download Step Options
   URL https://github.com/TA-Lib/ta-lib/archive/refs/tags/v0.7.1.tar.gz
   URL_HASH SHA256=40e7a6978052fe5245771e430e6a4c4553b40038f8ac5a985a1540c4c1fa6ace
   DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(talib)
organize_thirdparty_directory_targets("${talib_SOURCE_DIR}" thirdparty)
