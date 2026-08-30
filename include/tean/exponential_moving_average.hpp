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

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

template<uint32_t period = static_cast<uint32_t>(-1), uint32_t untrusted_period = period>
class exponential_moving_average;

template<uint32_t period, uint32_t untrusted_period>
class [[maybe_unused]] exponential_moving_average
{
   static_assert(1u < period);
   static_assert(static_cast<uint32_t>(-1) != period);
   static_assert(static_cast<uint32_t>(-1) != untrusted_period);

public:
   static constexpr inline auto lookback_period{untrusted_period + period - 1u,};

   exponential_moving_average(exponential_moving_average &&) = delete;
   exponential_moving_average(exponential_moving_average const &) = delete;

   [[maybe_unused, nodiscard]] constexpr explicit exponential_moving_average(double const inSmoothing = 2e0) noexcept :
      m_smoothingFactor(inSmoothing / (period + 1u))
   {
      assert(true == std::isfinite(m_smoothingFactor));
   }

   exponential_moving_average &operator = (exponential_moving_average &&) = delete;
   exponential_moving_average &operator = (exponential_moving_average const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
      m_prevSequenceNumber = inSequenceNumber;
#endif
      assert(true == std::isfinite(inValue));
      if (period <= inSequenceNumber) [[likely]]
      {
         m_value += m_smoothingFactor * (inValue - m_value);
         if (lookback_period <= inSequenceNumber) [[likely]]
         {
            return m_value;
         }
      }
      else
      {
         m_value += inValue;
         if (period == (inSequenceNumber + 1ull))
         {
            m_value /= period;
            if (lookback_period == inSequenceNumber)
            {
               return m_value;
            }
         }
      }
      return std::numeric_limits<double>::signaling_NaN();
   }

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
#endif
      assert(true == std::isfinite(inValue));
      if ((period <= inSequenceNumber) && (lookback_period <= inSequenceNumber)) [[likely]]
      {
         return m_value + m_smoothingFactor * (inValue - m_value);
      }
      if ((period == (inSequenceNumber + 1ull)) && (lookback_period == inSequenceNumber))
      {
         return (m_value + inValue) / period;
      }
      return std::numeric_limits<double>::signaling_NaN();
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      m_value = 0e0;
#if (not defined(NDEBUG))
      m_prevSequenceNumber = 0ull;
#endif
   }

private:
   double const m_smoothingFactor;
   double m_value{0e0,};
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0ull,};
#endif
};

template<>
class [[maybe_unused]] exponential_moving_average<static_cast<uint32_t>(-1), static_cast<uint32_t>(-1)> final
{
public:
   exponential_moving_average() = delete;
   exponential_moving_average(exponential_moving_average &&) = delete;
   exponential_moving_average(exponential_moving_average const &) = delete;
   [[nodiscard]] exponential_moving_average(uint32_t inPeriod, uint32_t inUntrustedPeriod, double inSmoothing = 2e0) noexcept;

   exponential_moving_average &operator = (exponential_moving_average &&) = delete;
   exponential_moving_average &operator = (exponential_moving_average const &) = delete;

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

   [[maybe_unused]] void reset() noexcept
   {
      m_value = 0e0;
#if (not defined(NDEBUG))
      m_prevSequenceNumber = 0ull;
#endif
   }

   [[maybe_unused, nodiscard]] double value() const noexcept
   {
      return m_value;
   }

private:
   uint32_t const m_period;
   uint32_t const m_lookbackPeriod;
   double const m_smoothingFactor;
   double m_value{0e0,};
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0ull,};
#endif
};

}
