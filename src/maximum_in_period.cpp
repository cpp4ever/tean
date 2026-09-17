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

#include "tean/maximum_in_period.hpp" /// for tean::maximum_in_period

#include <algorithm> /// for std::ranges::fill, std::max
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::make_unique
#include <ranges> /// for std::views::iota
#include <span> /// for std::span
#include <tuple> /// for std::tie
#include <utility> /// for std::pair

namespace tean
{

maximum_in_period::maximum_in_period(uint32_t const inPeriod) :
   m_period{inPeriod,},
   m_lookbackPeriod{inPeriod - 1u,},
   m_values{std::make_unique<double[]>(inPeriod),}
{
   assert(1u < period());
   std::ranges::fill(std::span{m_values.get(), period(),}, std::numeric_limits<double>::signaling_NaN());
}

double maximum_in_period::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(true == m_sequenceChecker.calc(inSequenceNumber));
#endif
   assert(true == std::isfinite(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      auto const valueIndex{static_cast<uint32_t>(inSequenceNumber % period()),};
      double maximumValue;
      std::tie(m_maximumValueIndex, maximumValue) = get_maximum(valueIndex, inValue);
      m_values[valueIndex] = inValue;
      return maximumValue;
   }
   m_values[inSequenceNumber] = inValue;
   auto const prevMaximumValue{m_values[m_maximumValueIndex],};
   if (prevMaximumValue <= inValue)
   {
      m_maximumValueIndex = static_cast<uint32_t>(inSequenceNumber);
      return inValue;
   }
   return prevMaximumValue;
}

double maximum_in_period::pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
{
#if (not defined(NDEBUG))
   assert(true == m_sequenceChecker.pick(inSequenceNumber));
#endif
   assert(true == std::isfinite(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      return get_maximum(static_cast<uint32_t>(inSequenceNumber % period()), inValue).second;
   }
   return (0ull == inSequenceNumber) ? inValue : std::max(m_values[m_maximumValueIndex], inValue);
}

void maximum_in_period::reset() noexcept
{
   std::ranges::fill(std::span{m_values.get(), period(),}, std::numeric_limits<double>::signaling_NaN());
   m_maximumValueIndex = 0u;
#if (not defined(NDEBUG))
   m_sequenceChecker.reset();
#endif
}

std::pair<uint32_t, double> maximum_in_period::get_maximum(uint32_t const inIndex, double const inValue) const noexcept
{
   auto const prevMaximumValue{m_values[m_maximumValueIndex],};
   if (prevMaximumValue <= inValue)
   {
      return std::make_pair(inIndex, inValue);
   }
   if (inIndex == m_maximumValueIndex)
   {
      auto maximumValueIndex{inIndex,};
      auto maximumValue{inValue,};
      for (auto const valueIndex : std::views::iota(0u, period()))
      {
         auto const value{m_values[valueIndex],};
         if ((maximumValue < value) && (inIndex != valueIndex))
         {
            maximumValueIndex = valueIndex;
            maximumValue = value;
         }
      }
      return std::make_pair(maximumValueIndex, maximumValue);
   }
   return std::make_pair(m_maximumValueIndex, prevMaximumValue);
}

}
