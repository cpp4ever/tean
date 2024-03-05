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

namespace tean
{

class [[nodiscard]] relative_strength_index final
{
public:
   relative_strength_index() = delete;
   relative_strength_index(relative_strength_index &&) = delete;
   relative_strength_index(relative_strength_index const &) = delete;
   [[nodiscard]] explicit relative_strength_index(uint32_t inPeriod) noexcept;

   relative_strength_index &operator = (relative_strength_index &&) = delete;
   relative_strength_index &operator = (relative_strength_index const &) = delete;

   [[nodiscard]] double calc(uint64_t inSequenceNumber, double inValue) noexcept;

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_period;
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_period;
   }

   [[nodiscard]] double pick(uint64_t inSequenceNumber, double inValue) const noexcept;

   void reset() noexcept;

private:
   uint32_t const m_period;
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber;
#endif
   double m_smoothGain;
   double m_smoothLoss;
   double m_value;

   [[nodiscard]] double do_lookback_calc(uint64_t inSequenceNumber, double inValue) noexcept;

   [[nodiscard]] double do_lookback_pick(double inValue) const noexcept;

   [[nodiscard]] double do_regular_calc(double inValue) noexcept;

   [[nodiscard]] double do_regular_pick(double inValue) const noexcept;
};

}
