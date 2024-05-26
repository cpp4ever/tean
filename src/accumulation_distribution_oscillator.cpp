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

#include "tean/accumulation_distribution_oscillator.hpp" /// for tean::accumulation_distribution_oscillator

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

accumulation_distribution_oscillator::accumulation_distribution_oscillator(uint32_t const inFastPeriod, uint32_t const inSlowPeriod, uint32_t const inUntrustedPeriod) noexcept :
   m_period(inSlowPeriod),
   m_lookbackPeriod(inUntrustedPeriod + inSlowPeriod - 1),
   m_fastSmoothingFactor(2.0 / static_cast<double>(inFastPeriod + 1)),
   m_slowSmoothingFactor(2.0 / static_cast<double>(inSlowPeriod + 1)),
   m_accumulationDistributionLine(),
   m_fastMovingAverage(0.0),
   m_slowMovingAverage(0.0)
{
   assert(1 < inFastPeriod);
   assert(inFastPeriod < inSlowPeriod);
   assert((inSlowPeriod - 1) <= lookback_period());
}

double accumulation_distribution_oscillator::do_calc(uint64_t const inSequenceNumber, double const inAccumulationDistribution) noexcept
{
   assert(true == std::isfinite(inAccumulationDistribution));
   assert(false == std::isnan(inAccumulationDistribution));
   if (0 < inSequenceNumber) [[likely]]
   {
      m_fastMovingAverage += m_fastSmoothingFactor * (inAccumulationDistribution - m_fastMovingAverage);
      m_slowMovingAverage += m_slowSmoothingFactor * (inAccumulationDistribution - m_slowMovingAverage);
      if (lookback_period() <= inSequenceNumber) [[likely]]
      {
         return m_fastMovingAverage - m_slowMovingAverage;
      }
   }
   else
   {
      m_fastMovingAverage = inAccumulationDistribution;
      m_slowMovingAverage = inAccumulationDistribution;
   }
   return std::numeric_limits<double>::signaling_NaN();
}

double accumulation_distribution_oscillator::do_pick(uint64_t const inSequenceNumber, double const inAccumulationDistribution) const noexcept
{
   assert(true == std::isfinite(inAccumulationDistribution));
   assert(false == std::isnan(inAccumulationDistribution));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      auto const fastMovingAverage = m_fastMovingAverage + m_fastSmoothingFactor * (inAccumulationDistribution - m_fastMovingAverage);
      auto const slowMovingAverage = m_slowMovingAverage + m_slowSmoothingFactor * (inAccumulationDistribution - m_slowMovingAverage);
      return fastMovingAverage - slowMovingAverage;
   }
   return std::numeric_limits<double>::signaling_NaN();
}

}
