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
#include <cstdint> /// for int32_t, uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::unique_ptr

namespace tean
{

struct linear_regression_result final
{
   double intercept{std::numeric_limits<double>::signaling_NaN(),};
   double slope{std::numeric_limits<double>::signaling_NaN(),};
};

template<uint32_t period = static_cast<uint32_t>(-1)>
class linear_regression;

template<uint32_t period>
class [[maybe_unused]] linear_regression final
{
   static_assert(1 < period);

public:
   static constexpr inline auto lookback_period{period - 1,};

private:
   static constexpr inline auto sum_x{period * lookback_period * 0.5,};
   static constexpr inline auto sum_square_x{(period * lookback_period * (2 * period - 1)) / 6.0,};
   static constexpr inline auto divisor{sum_x * sum_x - period * sum_square_x,};

public:
   [[maybe_unused, nodiscard]] constexpr linear_regression() noexcept
   {
#if (not defined(NDEBUG))
      std::ranges::fill(m_yValues, std::numeric_limits<double>::signaling_NaN());
#endif
   }

   linear_regression(linear_regression &&) = delete;
   linear_regression(linear_regression const &) = delete;

   linear_regression &operator = (linear_regression &&) = delete;
   linear_regression &operator = (linear_regression const &) = delete;

   [[maybe_unused, nodiscard]] constexpr linear_regression_result calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
      m_prevSequenceNumber = inSequenceNumber;
#endif
      assert(true == std::isfinite(inValue));
      m_yValues[inSequenceNumber % period] = inValue;
      if (lookback_period <= inSequenceNumber) [[likely]]
      {
         return do_calc(inSequenceNumber);
      }
      return linear_regression_result{};
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
#if (not defined(NDEBUG))
      std::ranges::fill(m_yValues, std::numeric_limits<double>::signaling_NaN());
      m_prevSequenceNumber = 0;
#endif
   }

private:
   std::array<double, period> m_yValues{};
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0,};
#endif

   [[nodiscard]] constexpr linear_regression_result do_calc(uint64_t const inSequenceNumber) const noexcept
   {
      double sumY{0,};
      double sumXY{0,};
      for (auto x{static_cast<int32_t>(period),}; 0 != x--; )
      {
         auto const y{m_yValues[(inSequenceNumber - x) % period],};
         sumY += y;
         sumXY += x * y;
      }
      auto const slope{(period * sumXY - sum_x * sumY) / divisor,};
      return linear_regression_result
      {
         .intercept = (sumY - slope * sum_x) / period,
         .slope = slope,
      };
   }
};

template<>
class [[maybe_unused]] linear_regression<static_cast<uint32_t>(-1)> final
{
public:
   linear_regression() = delete;
   linear_regression(linear_regression &&) = delete;
   linear_regression(linear_regression const &) = delete;
   [[nodiscard]] explicit linear_regression(uint32_t inPeriod);

   linear_regression &operator = (linear_regression &&) = delete;
   linear_regression &operator = (linear_regression const &) = delete;

   [[nodiscard]] linear_regression_result calc(uint64_t inSequenceNumber, double inValue) noexcept;

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
   std::unique_ptr<double[]> const m_yValues;
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0,};
#endif

   [[nodiscard]] linear_regression_result do_calc(uint64_t inSequenceNumber) noexcept;
};

}
