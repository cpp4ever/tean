/*
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
*/

#pragma once

#include <array> /// for std::array
#include <cstdint> /// for int64_t, uint64_t, uint8_t

namespace tean::tests
{

[[maybe_unused]] constexpr inline std::array<uint64_t, 20ui32> power_of_ten
{
   {
      1ui64,
      10ui64,
      100ui64,
      1000ui64,
      10000ui64,
      100000ui64,
      1000000ui64,
      10000000ui64,
      100000000ui64,
      1000000000ui64,
      10000000000ui64,
      100000000000ui64,
      1000000000000ui64,
      10000000000000ui64,
      100000000000000ui64,
      1000000000000000ui64,
      10000000000000000ui64,
      100000000000000000ui64,
      1000000000000000000ui64,
      10000000000000000000ui64,
   },
};

[[maybe_unused]] constexpr inline std::array<double, 20ui32> inverted_power_of_ten
{
   {
      1e-00,
      1e-01,
      1e-02,
      1e-03,
      1e-04,
      1e-05,
      1e-06,
      1e-07,
      1e-08,
      1e-09,
      1e-10,
      1e-11,
      1e-12,
      1e-13,
      1e-14,
      1e-15,
      1e-16,
      1e-17,
      1e-18,
      1e-19,
   },
};

struct [[maybe_unused]] decimal
{
   int64_t value{0ui64,};
   uint8_t scale{0ui8,};

   [[maybe_unused, nodiscard]] explicit operator double() const
   {
      return value * inverted_power_of_ten[scale];
   }
};

}
