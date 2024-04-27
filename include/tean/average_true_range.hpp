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

#include "tean/true_range.hpp" /// for tean::true_range

#include <cstdint> /// for uint32_t, uint64_t

namespace tean
{

class [[nodiscard]] average_true_range final
{
public:
   average_true_range() = delete;
   average_true_range(average_true_range &&) = delete;
   average_true_range(average_true_range const &) = delete;
   [[nodiscard]] average_true_range(uint32_t inPeriod, uint32_t inUntrustedPeriod) noexcept;

   average_true_range &operator = (average_true_range &&) = delete;
   average_true_range &operator = (average_true_range const &) = delete;

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) noexcept
   {
      return do_calc(inSequenceNumber, m_trueRange.calc(inSequenceNumber, inHigh, inLow, inClose));
   }

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double &outTrueRange) noexcept
   {
      outTrueRange = m_trueRange.calc(inSequenceNumber, inHigh, inLow, inClose);
      return do_calc(inSequenceNumber, outTrueRange);
   }

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_lookbackPeriod;
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_period;
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) const noexcept
   {
      return do_pick(inSequenceNumber, m_trueRange.pick(inSequenceNumber, inHigh, inLow, inClose));
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double &outTrueRange) const noexcept
   {
      outTrueRange = m_trueRange.pick(inSequenceNumber, inHigh, inLow, inClose);
      return do_pick(inSequenceNumber, outTrueRange);
   }

   [[maybe_unused]] void reset() noexcept
   {
      m_trueRange.reset();
      m_value = 0.0;
   }

private:
   uint32_t const m_period;
   uint32_t const m_lookbackPeriod;
   true_range m_trueRange;
   double m_value;

   [[nodiscard]] double do_calc(uint64_t inSequenceNumber, double inTrueRange) noexcept;

   [[nodiscard]] double do_pick(uint64_t inSequenceNumber, double inTrueRange) const noexcept;
};

}
