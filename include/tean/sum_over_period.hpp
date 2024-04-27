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

#pragma once

#include <cstdint> /// for uint32_t, uint64_t
#include <memory> /// for std::unique_ptr

namespace tean
{

class [[nodiscard]] sum_over_period final
{
public:
   sum_over_period() = delete;
   sum_over_period(sum_over_period &&) = delete;
   sum_over_period(sum_over_period const &) = delete;
   [[nodiscard]] explicit sum_over_period(uint32_t inPeriod);

   sum_over_period &operator = (sum_over_period &&) = delete;
   sum_over_period &operator = (sum_over_period const &) = delete;

   [[nodiscard]] double calc(uint64_t inSequenceNumber, double inValue) noexcept;

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_lookbackPeriod;
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_period;
   }

   [[nodiscard]] double pick(uint64_t inSequenceNumber, double inValue) const noexcept;

   void reset() noexcept;

private:
   uint32_t const m_period;
   uint32_t const m_lookbackPeriod;
   std::unique_ptr<double[]> const m_values;
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber;
#endif
   double m_sum;
};

}
