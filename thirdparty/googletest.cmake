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

set(BUILD_GMOCK ON CACHE BOOL "Enable googlemock targets" FORCE)
set(GTEST_HAS_ABSL OFF CACHE BOOL "Disable Abseil and RE2 dependencies" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Disable install targets" FORCE)
FetchContent_Declare(
   googletest
   # Download Step Options
   GIT_PROGRESS ON
   GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
   GIT_REPOSITORY https://github.com/google/googletest.git
   GIT_SHALLOW ON
   GIT_SUBMODULES_RECURSE ON
   GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)
set_property(DIRECTORY "${googletest_SOURCE_DIR}" PROPERTY EXCLUDE_FROM_ALL ON)
set_target_properties(gmock_main PROPERTIES FOLDER thirdparty)
