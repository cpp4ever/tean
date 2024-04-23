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

#include "tean/linear_regression.hpp" /// for tean::linear_regression

#include <algorithm> /// for std::fill
#include <cassert> /// for assert
#include <cmath> /// for std::isnan
#include <cstdint> /// for int32_t, uint32_t, uint64_t
#include <memory> /// for std::make_unique

namespace tean
{

namespace
{

static double period_to_sum_x(uint32_t const inPeriod) noexcept
{
   return static_cast<double>(inPeriod * (inPeriod - 1)) * 0.5;
}

static double period_to_sum_square_x(uint32_t const inPeriod) noexcept
{
   return static_cast<double>(inPeriod * (inPeriod - 1) * (2 * inPeriod - 1)) / 6.0;
}

}

linear_regression::linear_regression(uint32_t const inPeriod) :
   m_period(inPeriod),
   m_lookbackPeriod(inPeriod - 1),
   m_sumX(period_to_sum_x(inPeriod)),
   m_divisor(period_to_sum_x(inPeriod) * period_to_sum_x(inPeriod) - static_cast<double>(inPeriod) * period_to_sum_square_x(inPeriod)),
   m_yValues(std::make_unique<double[]>(inPeriod))
#if (not defined(NDEBUG))
   , m_prevSequenceNumber(0)
#endif
{
   assert(1 < period());
   std::fill(m_yValues.get(), m_yValues.get() + period(), 0.0);
}

linear_regression::result_type linear_regression::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(false == std::isnan(inValue));
   m_yValues[inSequenceNumber % period()] = inValue;
   if (lookback_period() <= inSequenceNumber) [[likely]]
   {
      return do_calc(inSequenceNumber);
   }
   return result_type{};
}

void linear_regression::reset() noexcept
{
   std::fill(m_yValues.get(), m_yValues.get() + period(), 0.0);
#if (not defined(NDEBUG))
   m_prevSequenceNumber = 0;
#endif
}

linear_regression::result_type linear_regression::do_calc(uint64_t const inSequenceNumber) noexcept
{
   auto sumY = 0.0;
   auto sumXY = 0.0;
   for (auto x = static_cast<int32_t>(period()); 0 != x--; )
   {
      auto const y = m_yValues[(inSequenceNumber - x) % period()];
      sumY += y;
      sumXY += static_cast<double>(x) * y;
   }
   auto const slope = (static_cast<double>(period()) * sumXY - m_sumX * sumY) / m_divisor;
   auto const intercept = (sumY - slope * m_sumX) / static_cast<double>(period());
   return result_type
   {
      .intercept = intercept,
      .slope = slope,
   };
}

}
