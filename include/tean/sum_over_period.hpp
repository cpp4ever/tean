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

#pragma once

#include <algorithm> /// for std::ranges::fill
#include <array> /// for std::array
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <memory> /// for std::unique_ptr

namespace tean
{

template<uint32_t period = static_cast<uint32_t>(-1)>
class sum_over_period;

template<uint32_t period>
class [[maybe_unused]] sum_over_period
{
   static_assert(1 < period);
   static_assert(static_cast<uint32_t>(-1) != period);

public:
   static constexpr inline auto lookback_period{period - 1u,};

   [[maybe_unused, nodiscard]] constexpr explicit sum_over_period() noexcept
   {
      std::ranges::fill(m_values, 0e0);
   }

   sum_over_period(sum_over_period &&) = delete;
   sum_over_period(sum_over_period const &) = delete;

   sum_over_period &operator = (sum_over_period &&) = delete;
   sum_over_period &operator = (sum_over_period const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
      m_prevSequenceNumber = inSequenceNumber;
#endif
      assert(true == std::isfinite(inValue));
      auto &prevValue{m_values[inSequenceNumber % period],};
      m_sum += inValue - prevValue;
      prevValue = inValue;
      return m_sum;
   }

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
#endif
      assert(true == std::isfinite(inValue));
      return inValue - m_values[inSequenceNumber % period] + m_sum;
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      std::ranges::fill(m_values, 0e0);
      m_sum = 0e0;
#if (not defined(NDEBUG))
      m_prevSequenceNumber = 0ull;
#endif
   }

private:
   std::array<double, period> m_values{};
   double m_sum{0e0,};
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0ull,};
#endif
};

template<>
class [[maybe_unused]] sum_over_period<static_cast<uint32_t>(-1)> final
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
   double m_sum{0e0,};
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0ull,};
#endif
};

}
