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

#include "tean/standard_deviation.hpp" /// for tean::standard_deviation

#include <cassert> /// for assert
#include <cmath> /// for std::isnan, std::sqrt
#include <cstdint> /// for uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

double standard_deviation::variance_to_standard_deviation(uint64_t const inSequenceNumber, double const inVariance) const noexcept
{
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      assert(false == std::isnan(inVariance));
      return (0.0 >= inVariance) ? 0.0 : std::sqrt(inVariance);
   }
   assert(true == std::isnan(inVariance));
   return std::numeric_limits<double>::quiet_NaN();
}

}
