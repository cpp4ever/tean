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

class [[nodiscard]] true_range final
{
public:
   [[maybe_unused, nodiscard]] true_range() noexcept :
#if (not defined(NDEBUG))
      m_prevSequenceNumber(0),
#endif
      m_lastClose(0.0)
   {}

   true_range(true_range &&) = delete;
   true_range(true_range const &) = delete;

   true_range &operator = (true_range &&) = delete;
   true_range &operator = (true_range const &) = delete;

   [[nodiscard]] double calc(uint64_t inSequenceNumber, double inHigh, double inLow, double inClose) noexcept;

   [[maybe_unused, nodiscard]] static constexpr uint32_t lookback_period() noexcept
   {
      return 1;
   }

   [[nodiscard]] double pick(uint64_t inSequenceNumber, double inHigh, double inLow, double inClose) const noexcept;

   [[maybe_unused]] void reset() noexcept
   {
#if (not defined(NDEBUG))
      m_prevSequenceNumber = 0;
#endif
      m_lastClose = 0.0;
   }

private:
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber;
#endif
   double m_lastClose;

   [[nodiscard]] double do_pick(uint64_t inSequenceNumber, double inHigh, double inLow) const noexcept;
};

}
