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

#include "tean/linear_regression.hpp" /// for tean::linear_regression

#include <algorithm> /// for std::ranges::fill
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for int32_t, uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::make_unique
#include <span> /// for std::span

namespace tean
{

namespace
{

static double period_to_sum_x(uint32_t const inPeriod) noexcept
{
   return inPeriod * (inPeriod - 1) * 0.5;
}

static double period_to_sum_square_x(uint32_t const inPeriod) noexcept
{
   return (inPeriod * (inPeriod - 1) * (2 * inPeriod - 1)) / 6.0;
}

}

linear_regression<static_cast<uint32_t>(-1)>::linear_regression(uint32_t const inPeriod) :
   m_period{inPeriod,},
   m_lookbackPeriod{inPeriod - 1,},
   m_sumX{period_to_sum_x(inPeriod),},
   m_divisor{period_to_sum_x(inPeriod) * period_to_sum_x(inPeriod) - inPeriod * period_to_sum_square_x(inPeriod),},
   m_yValues{std::make_unique<double[]>(inPeriod),}
{
   assert(1 < period());
#if (not defined(NDEBUG))
   std::ranges::fill(std::span{m_yValues.get(), period(),}, std::numeric_limits<double>::signaling_NaN());
#endif
}

linear_regression_result linear_regression<static_cast<uint32_t>(-1)>::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   m_yValues[inSequenceNumber % period()] = inValue;
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      return do_calc(inSequenceNumber);
   }
   return linear_regression_result{};
}

void linear_regression<static_cast<uint32_t>(-1)>::reset() noexcept
{
#if (not defined(NDEBUG))
   std::ranges::fill(std::span{m_yValues.get(), period(),}, std::numeric_limits<double>::signaling_NaN());
   m_prevSequenceNumber = 0;
#endif
}

linear_regression_result linear_regression<static_cast<uint32_t>(-1)>::do_calc(uint64_t const inSequenceNumber) noexcept
{
   double sumY{0,};
   double sumXY{0,};
   for (auto x{static_cast<int32_t>(period()),}; 0 != x--; )
   {
      auto const y{m_yValues[(inSequenceNumber - x) % period()],};
      sumY += y;
      sumXY += x * y;
   }
   auto const slope{(period() * sumXY - m_sumX * sumY) / m_divisor,};
   return linear_regression_result
   {
      .intercept = (sumY - slope * m_sumX) / period(),
      .slope = slope,
   };
}

}
