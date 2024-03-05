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
#include <utility> /// for std::pair

namespace tean
{

class [[nodiscard]] williams_percent_range final
{
public:
   williams_percent_range() = delete;
   williams_percent_range(williams_percent_range &&) = delete;
   williams_percent_range(williams_percent_range const &) = delete;
   [[nodiscard]] explicit williams_percent_range(uint32_t inPeriod);

   williams_percent_range &operator = (williams_percent_range &&) = delete;
   williams_percent_range &operator = (williams_percent_range const &) = delete;

   [[nodiscard]] double calc(uint64_t inSequenceNumber, double inHigh, double inLow, double inClose) noexcept;

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_lookbackPeriod;
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_period;
   }

   [[nodiscard]] double pick(uint64_t inSequenceNumber, double inHigh, double inLow, double inClose) const noexcept;

   void reset() noexcept;

private:
   uint32_t const m_period;
   uint32_t const m_lookbackPeriod;
   std::unique_ptr<double[]> const m_highValues;
   std::unique_ptr<double[]> const m_lowValues;
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber;
#endif
   uint32_t m_highestHighIndex;
   uint32_t m_lowestLowIndex;

   [[nodiscard]] double do_lookback_calc(uint64_t inSequenceNumber, double inHigh, double inLow, double inClose) noexcept;

   [[nodiscard]] double do_lookback_pick(double inHigh, double inLow, double inClose) const noexcept;

   [[nodiscard]] double do_regular_calc(uint64_t inSequenceNumber, double inHigh, double inLow, double inClose) noexcept;

   [[nodiscard]] double do_regular_pick(uint64_t inSequenceNumber, double inHigh, double inLow, double inClose) const noexcept;

   [[nodiscard]] std::pair<uint32_t, double> get_highest_high(uint32_t inIndex, double inValue) const noexcept;

   [[nodiscard]] std::pair<uint32_t, double> get_lowest_low(uint32_t inIndex, double inValue) const noexcept;
};

}
