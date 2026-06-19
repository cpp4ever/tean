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

#include "tean/variance.hpp" /// for tean::variance

#include <algorithm> /// for std::ranges::fill
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::make_unique
#include <span> /// for std::span

namespace tean
{

variance<static_cast<uint32_t>(-1)>::variance(uint32_t const inPeriod) :
   m_period(inPeriod),
   m_lookbackPeriod(inPeriod - 1),
   m_values(std::make_unique<double[]>(inPeriod - 1))
{
   assert(1 < period());
#if (not defined(NDEBUG))
   std::ranges::fill(std::span{m_values.get(), lookback_period(),}, std::numeric_limits<double>::signaling_NaN());
#endif
}

double variance<static_cast<uint32_t>(-1)>::calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      return do_regular_calc(inSequenceNumber, inValue, outMean);
   }
   do_lookback_calc(inSequenceNumber, inValue);
   outMean = std::numeric_limits<double>::signaling_NaN();
   return std::numeric_limits<double>::signaling_NaN();
}

double variance<static_cast<uint32_t>(-1)>::pick(uint64_t const inSequenceNumber, double const inValue, double &outMean) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inValue));
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      outMean = (m_sum + inValue) / period();
      auto const meanOfSquares{(m_sumOfSquares + inValue * inValue) / period(),};
      return meanOfSquares - outMean * outMean;
   }
   outMean = std::numeric_limits<double>::signaling_NaN();
   return std::numeric_limits<double>::signaling_NaN();
}

void variance<static_cast<uint32_t>(-1)>::reset() noexcept
{
   m_sum = 0;
   m_sumOfSquares = 0;
#if (not defined(NDEBUG))
   std::ranges::fill(std::span{m_values.get(), lookback_period(),}, std::numeric_limits<double>::signaling_NaN());
   m_prevSequenceNumber = 0;
#endif
}

void variance<static_cast<uint32_t>(-1)>::do_lookback_calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
   m_sum += inValue;
   m_sumOfSquares += inValue * inValue;
   m_values[inSequenceNumber % lookback_period()] = inValue;
}

double variance<static_cast<uint32_t>(-1)>::do_regular_calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
{
   m_sum += inValue;
   m_sumOfSquares += inValue * inValue;
   outMean = m_sum / period();
   auto const meanOfSquares{m_sumOfSquares / period(),};
   auto &prevValue{m_values[inSequenceNumber % lookback_period()],};
   m_sum -= prevValue;
   m_sumOfSquares -= prevValue * prevValue;
   prevValue = inValue;
   return meanOfSquares - outMean * outMean;
}

}
