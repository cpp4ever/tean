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

#include "tean/sum_over_period.hpp" /// for tean::sum_over_period

#include <algorithm> /// for std::ranges::fill
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <memory> /// for std::make_unique
#include <span> /// for std::span

namespace tean
{

sum_over_period<static_cast<uint32_t>(-1)>::sum_over_period(uint32_t const inPeriod) :
   m_period(inPeriod),
   m_lookbackPeriod(inPeriod - 1u),
   m_values(std::make_unique<double[]>(inPeriod))
{
   assert(1u < period());
   std::ranges::fill(std::span{m_values.get(), period(),}, 0e0);
}

double sum_over_period<static_cast<uint32_t>(-1)>::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   auto &prevValue{m_values[inSequenceNumber % period()],};
   m_sum += inValue - prevValue;
   prevValue = inValue;
   return m_sum;
}

double sum_over_period<static_cast<uint32_t>(-1)>::pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inValue));
   return inValue - m_values[inSequenceNumber % period()] + m_sum;
}

void sum_over_period<static_cast<uint32_t>(-1)>::reset() noexcept
{
   std::ranges::fill(std::span{m_values.get(), period(),}, 0e0);
   m_sum = 0e0;
#if (not defined(NDEBUG))
   m_prevSequenceNumber = 0ull;
#endif
}

}
