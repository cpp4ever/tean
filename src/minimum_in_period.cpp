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

#include "tean/minimum_in_period.hpp" /// for tean::minimum_in_period

#include <algorithm> /// for std::fill, std::min
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::make_unique
#include <tuple> /// for std::tie
#include <utility> /// for std::pair

namespace tean
{

minimum_in_period::minimum_in_period(uint32_t const inPeriod) :
   m_period(inPeriod),
   m_lookbackPeriod(inPeriod - 1),
   m_values(std::make_unique<double[]>(inPeriod)),
#if (not defined(NDEBUG))
   m_prevSequenceNumber(0),
#endif
   m_minimumValueIndex(0)
{
   assert(1 < period());
   std::fill(m_values.get(), m_values.get() + period(), std::numeric_limits<double>::signaling_NaN());
}

double minimum_in_period::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      auto const valueIndex = static_cast<uint32_t>(inSequenceNumber % period());
      double minimumValue;
      std::tie(m_minimumValueIndex, minimumValue) = get_minimum(valueIndex, inValue);
      m_values[valueIndex] = inValue;
      return minimumValue;
   }
   m_values[inSequenceNumber] = inValue;
   auto const prevMinimumValue = m_values[m_minimumValueIndex];
   if (inValue <= prevMinimumValue)
   {
      m_minimumValueIndex = static_cast<uint32_t>(inSequenceNumber);
      return inValue;
   }
   return prevMinimumValue;
}

double minimum_in_period::pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      return get_minimum(static_cast<uint32_t>(inSequenceNumber % period()), inValue).second;
   }
   return (0 == inSequenceNumber)
      ? inValue
      : std::min(m_values[m_minimumValueIndex], inValue)
   ;
}

void minimum_in_period::reset() noexcept
{
   std::fill(m_values.get(), m_values.get() + period(), std::numeric_limits<double>::signaling_NaN());
#if (not defined(NDEBUG))
   m_prevSequenceNumber = 0;
#endif
   m_minimumValueIndex = 0;
}

std::pair<uint32_t, double> minimum_in_period::get_minimum(uint32_t const inIndex, double const inValue) const noexcept
{
   auto const prevMinimumValue = m_values[m_minimumValueIndex];
   if (inValue <= prevMinimumValue)
   {
      return {inIndex, inValue};
   }
   if (inIndex == m_minimumValueIndex)
   {
      auto minimumValueIndex = inIndex;
      auto minimumValue = inValue;
      for (uint32_t valueIndex = 0; valueIndex < period(); ++valueIndex)
      {
         auto const value = m_values[valueIndex];
         if ((value < minimumValue) && (inIndex != valueIndex))
         {
            minimumValueIndex = valueIndex;
            minimumValue = value;
         }
      }
      return {minimumValueIndex, minimumValue};
   }
   return {m_minimumValueIndex, prevMinimumValue};
}

}
