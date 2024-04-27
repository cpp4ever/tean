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

#include "tean/variance.hpp" /// for tean::variance

#include <cstdint> /// for uint32_t, uint64_t

namespace tean
{

class [[nodiscard]] standard_deviation final
{
public:
   standard_deviation() = delete;
   standard_deviation(standard_deviation &&) = delete;
   standard_deviation(standard_deviation const &) = delete;

   [[maybe_unused, nodiscard]] explicit standard_deviation(uint32_t const inPeriod) noexcept :
      m_variance(inPeriod)
   {}

   standard_deviation &operator = (standard_deviation &&) = delete;
   standard_deviation &operator = (standard_deviation const &) = delete;

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.calc(inSequenceNumber, inValue));
   }

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.calc(inSequenceNumber, inValue, outMean));
   }

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_variance.lookback_period();
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_variance.period();
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.pick(inSequenceNumber, inValue));
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inValue, double &outMean) const noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.pick(inSequenceNumber, inValue, outMean));
   }

   [[maybe_unused]] void reset() noexcept
   {
      m_variance.reset();
   }

private:
   variance m_variance;

   [[nodiscard]] double variance_to_standard_deviation(uint64_t inSequenceNumber, double inVariance) const noexcept;
};

}
