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

class [[nodiscard]] linear_regression final
{
public:
   struct [[maybe_unused, nodiscard]] result_type final
   {
      [[maybe_unused]] double intercept;
      [[maybe_unused]] double slope;
   };

public:
   linear_regression() = delete;
   linear_regression(linear_regression &&) = delete;
   linear_regression(linear_regression const &) = delete;
   [[nodiscard]] explicit linear_regression(uint32_t inPeriod);

   linear_regression &operator = (linear_regression &&) = delete;
   linear_regression &operator = (linear_regression const &) = delete;

   [[nodiscard]] result_type calc(uint64_t inSequenceNumber, double inValue) noexcept;

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_lookbackPeriod;
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_period;
   }

   void reset() noexcept;

private:
   uint32_t const m_period;
   uint32_t const m_lookbackPeriod;
   double const m_sumX;
   double const m_divisor;
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber;
#endif
   std::unique_ptr<double[]> const m_yValues;

   [[nodiscard]] result_type do_calc(uint64_t inSequenceNumber) noexcept;
};

}
