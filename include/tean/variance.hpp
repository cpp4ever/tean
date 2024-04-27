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

class [[nodiscard]] variance final
{
public:
   variance() = delete;
   variance(variance &&) = delete;
   variance(variance const &) = delete;
   [[nodiscard]] explicit variance(uint32_t inPeriod);

   variance &operator = (variance &&) = delete;
   variance &operator = (variance const &) = delete;

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      double mean;
      return calc(inSequenceNumber, inValue, mean);
   }

   [[nodiscard]] double calc(uint64_t inSequenceNumber, double inValue, double &outMean) noexcept;

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_lookbackPeriod;
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_period;
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
      double mean;
      return pick(inSequenceNumber, inValue, mean);
   }

   [[nodiscard]] double pick(uint64_t inSequenceNumber, double inValue, double &outMean) const noexcept;

   void reset() noexcept;

private:
   uint32_t const m_period;
   uint32_t const m_lookbackPeriod;
   std::unique_ptr<double[]> const m_values;
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber;
#endif
   double m_sum;
   double m_sumOfSquares;

   [[nodiscard]] double do_lookback_calc(uint64_t inSequenceNumber, double inValue) noexcept;

   [[nodiscard]] double do_regular_calc(uint64_t inSequenceNumber, double inValue, double &outMean) noexcept;
};

}
