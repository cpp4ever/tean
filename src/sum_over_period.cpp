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

#include "tean/sum_over_period.hpp" /// for tean::sum_over_period

#include <algorithm> /// for std::fill
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint32_t, uint64_t
#include <memory> /// for std::make_unique

namespace tean
{

sum_over_period::sum_over_period(uint32_t const inPeriod) :
   m_period(inPeriod),
   m_lookbackPeriod(inPeriod - 1),
   m_values(std::make_unique<double[]>(inPeriod)),
#if (not defined(NDEBUG))
   m_prevSequenceNumber(0),
#endif
   m_sum(0.0)
{
   assert(1 < period());
   std::fill(m_values.get(), m_values.get() + period(), 0.0);
}

double sum_over_period::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   auto &prevValue = m_values[inSequenceNumber % period()];
   m_sum += inValue - prevValue;
   prevValue = inValue;
   return m_sum;
}

double sum_over_period::pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   return inValue - m_values[inSequenceNumber % period()] + m_sum;
}

void sum_over_period::reset() noexcept
{
   std::fill(m_values.get(), m_values.get() + period(), 0.0);
#if (not defined(NDEBUG))
   m_prevSequenceNumber = 0;
#endif
   m_sum = 0.0;
}

}
