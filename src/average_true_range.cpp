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

#include "tean/average_true_range.hpp" /// for tean::average_true_range
#include "tean/true_range.hpp" /// for tean::true_range

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

average_true_range::average_true_range(uint32_t const inPeriod, uint32_t const inUntrustedPeriod) noexcept :
   m_period(inPeriod),
   m_lookbackPeriod(inUntrustedPeriod + inPeriod),
   m_trueRange(),
   m_value(0.0)
{
   assert(m_trueRange.lookback_period() < period());
}

double average_true_range::do_calc(uint64_t const inSequenceNumber, double const inTrueRange) noexcept
{
   if (m_trueRange.lookback_period() <= inSequenceNumber) [[likely]]
   {
      assert(true == std::isfinite(inTrueRange));
      assert(false == std::isnan(inTrueRange));
      if (period() < inSequenceNumber) [[likely]]
      {
         m_value = (m_value * static_cast<double>(period() - 1) + inTrueRange) / static_cast<double>(period());
         if (lookback_period() <= inSequenceNumber) [[likely]]
         {
            return m_value;
         }
      }
      else
      {
         m_value += inTrueRange;
         if (period() == inSequenceNumber) [[unlikely]]
         {
            m_value /= static_cast<double>(period());
            if (lookback_period() == inSequenceNumber)
            {
               return m_value;
            }
         }
      }
   }
   else
   {
      assert(true == std::isnan(inTrueRange));
   }
   return std::numeric_limits<double>::quiet_NaN();
}

double average_true_range::do_pick(uint64_t const inSequenceNumber, double const inTrueRange) const noexcept
{
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      assert(true == std::isfinite(inTrueRange));
      assert(false == std::isnan(inTrueRange));
      if (period() < inSequenceNumber) [[likely]]
      {
         return (m_value * static_cast<double>(period() - 1) + inTrueRange) / static_cast<double>(period());
      }
      assert(period() == lookback_period());
      return (m_value + inTrueRange) / static_cast<double>(period());
   }
   return std::numeric_limits<double>::quiet_NaN();
}

}
