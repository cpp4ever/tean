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

#include "tean/exponential_moving_average.hpp" /// for tean::exponential_moving_average

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

exponential_moving_average<static_cast<uint32_t>(-1), static_cast<uint32_t>(-1)>::exponential_moving_average(
   uint32_t const inPeriod,
   uint32_t const inUntrustedPeriod,
   double const inSmoothing
) noexcept :
   m_period(inPeriod),
   m_lookbackPeriod(inUntrustedPeriod + inPeriod - 1u),
   m_smoothingFactor(inSmoothing / (inPeriod + 1u))
{
   assert(true == std::isfinite(m_smoothingFactor));
   assert(1u < period());
}

double exponential_moving_average<static_cast<uint32_t>(-1), static_cast<uint32_t>(-1)>::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   if (period() <= inSequenceNumber) [[likely]]
   {
      m_value += m_smoothingFactor * (inValue - m_value);
      if (lookback_period() <= inSequenceNumber) [[likely]]
      {
         return m_value;
      }
   }
   else
   {
      m_value += inValue;
      if (period() == (inSequenceNumber + 1ull))
      {
         m_value /= period();
         if (lookback_period() == inSequenceNumber)
         {
            return m_value;
         }
      }
   }
   return std::numeric_limits<double>::signaling_NaN();
}

double exponential_moving_average<static_cast<uint32_t>(-1), static_cast<uint32_t>(-1)>::pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inValue));
   if ((period() <= inSequenceNumber) && (lookback_period() <= inSequenceNumber)) [[likely]]
   {
      return m_value + m_smoothingFactor * (inValue - m_value);
   }
   if ((period() == (inSequenceNumber + 1ull)) && (lookback_period() == inSequenceNumber))
   {
      return (m_value + inValue) / period();
   }
   return std::numeric_limits<double>::signaling_NaN();
}

}
