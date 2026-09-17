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

#include "tean/williams_percent_range.hpp" /// for tean::williams_percent_range

#include <algorithm> /// for std::fill
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::make_unique
#include <tuple> /// for std::tie
#include <utility> /// for std::pair

namespace tean
{

williams_percent_range::williams_percent_range(uint32_t const inPeriod) :
   m_period{inPeriod,},
   m_lookbackPeriod{inPeriod - 1u,},
   m_highValues{std::make_unique<double[]>(inPeriod),},
   m_lowValues{std::make_unique<double[]>(inPeriod),}
{
   assert(1u < period());
#if (not defined(NDEBUG))
   std::fill(m_highValues.get(), m_highValues.get() + period(), std::numeric_limits<double>::signaling_NaN());
   std::fill(m_lowValues.get(), m_lowValues.get() + period(), std::numeric_limits<double>::signaling_NaN());
#endif
}

double williams_percent_range::calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) noexcept
{
#if (not defined(NDEBUG))
   assert(true == m_sequenceChecker.calc(inSequenceNumber));
#endif
   assert(true == std::isfinite(inHigh));
   assert(true == std::isfinite(inLow));
   assert(true == std::isfinite(inClose));
   assert(inHigh >= inLow);
   assert(inHigh >= inClose);
   assert(inClose >= inLow);
   if (lookback_period() < inSequenceNumber) [[likely]]
   {
      return do_regular_calc(inSequenceNumber, inHigh, inLow, inClose);
   }
   return do_lookback_calc(inSequenceNumber, inHigh, inLow, inClose);
}

double williams_percent_range::pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) const noexcept
{
#if (not defined(NDEBUG))
   assert(true == m_sequenceChecker.pick(inSequenceNumber));
#endif
   assert(true == std::isfinite(inHigh));
   assert(true == std::isfinite(inLow));
   assert(true == std::isfinite(inClose));
   assert(inHigh >= inLow);
   assert(inHigh >= inClose);
   assert(inClose >= inLow);
   if (lookback_period() < inSequenceNumber) [[likely]]
   {
      return do_regular_pick(inSequenceNumber, inHigh, inLow, inClose);
   }
   if (lookback_period() == inSequenceNumber) [[unlikely]]
   {
      return do_lookback_pick(inHigh, inLow, inClose);
   }
   return std::numeric_limits<double>::signaling_NaN();
}

void williams_percent_range::reset() noexcept
{
#if (not defined(NDEBUG))
   std::fill(m_highValues.get(), m_highValues.get() + period(), std::numeric_limits<double>::signaling_NaN());
   std::fill(m_lowValues.get(), m_lowValues.get() + period(), std::numeric_limits<double>::signaling_NaN());
#endif
   m_highestHighIndex = 0u;
   m_lowestLowIndex = 0u;
#if (not defined(NDEBUG))
   m_sequenceChecker.reset();
#endif
}

double williams_percent_range::do_lookback_calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) noexcept
{
   auto const arrayIndex{static_cast<uint32_t>(inSequenceNumber % period()),};
   m_highValues[arrayIndex] = inHigh;
   m_lowValues[arrayIndex] = inLow;
   if (lookback_period() == inSequenceNumber) [[unlikely]]
   {
      m_highestHighIndex = 0u;
      auto highestHigh{m_highValues[0u],};
      for (auto highValuesIndex{1u,}; highValuesIndex < period(); ++highValuesIndex)
      {
         if (m_highValues[highValuesIndex] > highestHigh)
         {
            m_highestHighIndex = highValuesIndex;
            highestHigh = m_highValues[highValuesIndex];
         }
      }
      m_lowestLowIndex = 0u;
      auto lowestLow{m_lowValues[0u],};
      for (auto lowValuesIndex{1u,}; lowValuesIndex < period(); ++lowValuesIndex)
      {
         if (m_lowValues[lowValuesIndex] < lowestLow)
         {
            m_lowestLowIndex = lowValuesIndex;
            lowestLow = m_lowValues[lowValuesIndex];
         }
      }
      assert(highestHigh >= lowestLow);
      auto const delta{highestHigh - lowestLow,};
      return (0e0 == delta) ? 0e0 : (-100e0 * (highestHigh - inClose) / delta);
   }
   return std::numeric_limits<double>::signaling_NaN();
}

double williams_percent_range::do_lookback_pick(double const inHigh, double const inLow, double const inClose) const noexcept
{
   auto highestHigh{inHigh,};
   for (auto highValuesIndex{0u,}; highValuesIndex < lookback_period(); ++highValuesIndex)
   {
      if (m_highValues[highValuesIndex] > highestHigh)
      {
         highestHigh = m_highValues[highValuesIndex];
      }
   }
   auto lowestLow{inLow,};
   for (auto lowValuesIndex{0u,}; lowValuesIndex < lookback_period(); ++lowValuesIndex)
   {
      if (m_lowValues[lowValuesIndex] < lowestLow)
      {
         lowestLow = m_lowValues[lowValuesIndex];
      }
   }
   assert(highestHigh >= lowestLow);
   auto const delta{highestHigh - lowestLow,};
   return (0e0 == delta) ? 0e0 : (-100e0 * (highestHigh - inClose) / delta);
}

double williams_percent_range::do_regular_calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) noexcept
{
   double highestHigh;
   double lowestLow;
   {
      auto const arrayIndex{static_cast<uint32_t>(inSequenceNumber % period()),};
      std::tie(m_highestHighIndex, highestHigh) = get_highest_high(arrayIndex, inHigh);
      m_highValues[arrayIndex] = inHigh;
      std::tie(m_lowestLowIndex, lowestLow) = get_lowest_low(arrayIndex, inLow);
      m_lowValues[arrayIndex] = inLow;
   }
   assert(highestHigh >= lowestLow);
   auto const delta = (highestHigh - lowestLow);
   return (0e0 == delta) ? 0e0 : (-100e0 * (highestHigh - inClose) / delta);
}

double williams_percent_range::do_regular_pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) const noexcept
{
   auto const arrayIndex{static_cast<uint32_t>(inSequenceNumber % period()),};
   auto const highestHigh{get_highest_high(arrayIndex, inHigh).second,};
   auto const lowestLow{get_lowest_low(arrayIndex, inLow).second,};
   assert(highestHigh >= lowestLow);
   auto const delta{highestHigh - lowestLow,};
   return (0e0 == delta) ? 0e0 : (-100e0 * (highestHigh - inClose) / delta);
}

std::pair<uint32_t, double> williams_percent_range::get_highest_high(uint32_t const inIndex, double const inValue) const noexcept
{
   if (inValue >= m_highValues[m_highestHighIndex])
   {
      return {inIndex, inValue};
   }
   if (inIndex == m_highestHighIndex)
   {
      auto highestHighIndex{inIndex,};
      auto highestHigh{inValue,};
      for (auto highValuesIndex{0u,}; highValuesIndex < period(); ++highValuesIndex)
      {
         if ((inIndex != highValuesIndex) && (m_highValues[highValuesIndex] > highestHigh))
         {
            highestHighIndex = highValuesIndex;
            highestHigh = m_highValues[highValuesIndex];
         }
      }
      return {highestHighIndex, highestHigh,};
   }
   return {m_highestHighIndex, m_highValues[m_highestHighIndex],};
}

std::pair<uint32_t, double> williams_percent_range::get_lowest_low(uint32_t const inIndex, double const inValue) const noexcept
{
   if (inValue <= m_lowValues[m_lowestLowIndex])
   {
      return {inIndex, inValue,};
   }
   if (inIndex == m_lowestLowIndex)
   {
      auto lowestLowIndex{inIndex,};
      auto lowestLow{inValue,};
      for (auto lowValuesIndex{0u,}; lowValuesIndex < period(); ++lowValuesIndex)
      {
         if ((inIndex != lowValuesIndex) && (m_lowValues[lowValuesIndex] < lowestLow))
         {
            lowestLowIndex = lowValuesIndex;
            lowestLow = m_lowValues[lowValuesIndex];
         }
      }
      return {lowestLowIndex, lowestLow,};
   }
   return {m_lowestLowIndex, m_lowValues[m_lowestLowIndex],};
}

}
