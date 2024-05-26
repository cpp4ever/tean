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

#include "tean/variance.hpp" /// for tean::variance

#include <algorithm> /// for std::fill
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::make_unique

namespace tean
{

variance::variance(uint32_t const inPeriod) :
   m_period(inPeriod),
   m_lookbackPeriod(inPeriod - 1),
   m_values(std::make_unique<double[]>(inPeriod - 1)),
#if (not defined(NDEBUG))
   m_prevSequenceNumber(0),
#endif
   m_sum(0.0),
   m_sumOfSquares(0.0)
{
   assert(1 < period());
#if (not defined(NDEBUG))
   std::fill(m_values.get(), m_values.get() + lookback_period(), std::numeric_limits<double>::signaling_NaN());
#endif
}

double variance::calc(uint64_t inSequenceNumber, double inValue, double &outMean) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      return do_regular_calc(inSequenceNumber, inValue, outMean);
   }
   outMean = std::numeric_limits<double>::signaling_NaN();
   return do_lookback_calc(inSequenceNumber, inValue);
}

double variance::pick(uint64_t const inSequenceNumber, double const inValue, double &outMean) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      outMean = (m_sum + inValue) / static_cast<double>(period());
      auto const meanOfSquares = (m_sumOfSquares + inValue * inValue) / static_cast<double>(period());
      return meanOfSquares - outMean * outMean;
   }
   outMean = std::numeric_limits<double>::signaling_NaN();
   return std::numeric_limits<double>::signaling_NaN();
}

void variance::reset() noexcept
{
#if (not defined(NDEBUG))
   std::fill(m_values.get(), m_values.get() + lookback_period(), std::numeric_limits<double>::signaling_NaN());
   m_prevSequenceNumber = 0;
#endif
   m_sum = 0.0;
   m_sumOfSquares = 0.0;
}

double variance::do_lookback_calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
   m_sum += inValue;
   m_sumOfSquares += inValue * inValue;
   m_values[inSequenceNumber % lookback_period()] = inValue;
   return std::numeric_limits<double>::signaling_NaN();
}

double variance::do_regular_calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
{
   m_sum += inValue;
   m_sumOfSquares += inValue * inValue;
   outMean = m_sum / static_cast<double>(period());
   auto const meanOfSquares = m_sumOfSquares / static_cast<double>(period());
   auto &prevValue = m_values[inSequenceNumber % lookback_period()];
   m_sum -= prevValue;
   m_sumOfSquares -= prevValue * prevValue;
   prevValue = inValue;
   return meanOfSquares - outMean * outMean;
}

}
