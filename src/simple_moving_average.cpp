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

#include "tean/simple_moving_average.hpp" /// for tean::simple_moving_average

#include <cassert> /// for assert
#include <cmath> /// for std::isnan
#include <cstdint> /// for uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

double simple_moving_average::do_calc(uint64_t const inSequenceNumber, double const inSumOverPeriod) noexcept
{
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      assert(false == std::isnan(inSumOverPeriod));
      return inSumOverPeriod / static_cast<double>(period());
   }
   return std::numeric_limits<double>::quiet_NaN();
}

double simple_moving_average::do_pick(uint64_t const inSequenceNumber, double const inSumOverPeriod) const noexcept
{
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      assert(false == std::isnan(inSumOverPeriod));
      return inSumOverPeriod / static_cast<double>(period());
   }
   return std::numeric_limits<double>::quiet_NaN();
}

}
