/*
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
*/

#pragma once

#include <array> /// for std::array
#include <cstdint> /// for int64_t, uint64_t, uint8_t

namespace tean::tests
{

[[maybe_unused]] constexpr std::array<uint64_t, 20> power_of_ten
{
   {
      1ull,
      10ull,
      100ull,
      1000ull,
      10000ull,
      100000ull,
      1000000ull,
      10000000ull,
      100000000ull,
      1000000000ull,
      10000000000ull,
      100000000000ull,
      1000000000000ull,
      10000000000000ull,
      100000000000000ull,
      1000000000000000ull,
      10000000000000000ull,
      100000000000000000ull,
      1000000000000000000ull,
      10000000000000000000ull,
   },
};

[[maybe_unused]] constexpr std::array<double, 20> inverted_power_of_ten
{
   {
      1.0E-00,
      1.0E-01,
      1.0E-02,
      1.0E-03,
      1.0E-04,
      1.0E-05,
      1.0E-06,
      1.0E-07,
      1.0E-08,
      1.0E-09,
      1.0E-10,
      1.0E-11,
      1.0E-12,
      1.0E-13,
      1.0E-14,
      1.0E-15,
      1.0E-16,
      1.0E-17,
      1.0E-18,
      1.0E-19,
   },
};

struct [[maybe_unused, nodiscard]] decimal
{
   int64_t value = 0;
   uint8_t scale = 0;

   [[maybe_unused, nodiscard]] explicit operator double() const
   {
      return static_cast<double>(value) * inverted_power_of_ten[scale];
   }
};

}
