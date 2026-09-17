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

#include "tean/indicator_traits.hpp" ///< for tean::internals::indicator_traits, tean::lazy_indicator
#include "tean/sequence_checker.hpp" ///< for tean::sequence_checker

#include <algorithm> /// for std::ranges::fill
#include <array> /// for std::array
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::allocator
#include <ranges> /// for std::views::iota, std::views::reverse
#include <vector> /// for std::vector

namespace tean
{

namespace internals
{

template<uint32_t indicator_period>
struct linear_regression_options final
{};

template<>
struct linear_regression_options<lazy_indicator> final
{
   uint32_t const period;
   uint32_t const lookbackPeriod;
   double const sumX;
   double const divisor;
};

template<uint32_t indicator_period, typename y_values_allocator>
struct linear_regression_storage final
{
   std::array<double, indicator_period> yValues{};
};

template<typename y_values_allocator>
struct linear_regression_storage<lazy_indicator, y_values_allocator> final
{
   linear_regression_options<lazy_indicator> const options;
   std::vector<double, y_values_allocator> yValues;
};

[[nodiscard]] constexpr uint32_t linear_regression_lookback_period(uint32_t const inPeriod) noexcept
{
   return inPeriod - 1u;
}

[[nodiscard]] constexpr double linear_regression_sum_square_x(uint32_t const inPeriod) noexcept
{
   return (inPeriod * linear_regression_lookback_period(inPeriod) * (2u * inPeriod - 1u)) / 6e0;
}

[[nodiscard]] constexpr double linear_regression_sum_x(uint32_t const inPeriod) noexcept
{
   return inPeriod * linear_regression_lookback_period(inPeriod) * 5e-1;
}

[[nodiscard]] constexpr double linear_regression_divisor(uint32_t const inPeriod) noexcept
{
   return linear_regression_sum_x(inPeriod) * linear_regression_sum_x(inPeriod) - inPeriod * linear_regression_sum_square_x(inPeriod);
}

template<uint32_t indicator_period>
struct indicator_traits<linear_regression_options<indicator_period>> final
{
   using type = linear_regression_options<indicator_period>;

   [[nodiscard]] static constexpr double divisor() noexcept
   {
      return linear_regression_divisor(period());
   }

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept
   {
      return linear_regression_lookback_period(period());
   }

   [[nodiscard]] static constexpr uint32_t period() noexcept
   {
      return indicator_period;
   }

   [[nodiscard]] static constexpr double sum_x() noexcept
   {
      return linear_regression_sum_x(period());
   }
};

template<>
struct indicator_traits<linear_regression_options<lazy_indicator>> final
{
   using type = linear_regression_options<lazy_indicator>;

   [[nodiscard]] static constexpr double divisor(type const options) noexcept
   {
      return options.divisor;
   }

   [[nodiscard]] static constexpr uint32_t lookback_period(type const options) noexcept
   {
      return options.lookbackPeriod;
   }

   [[nodiscard]] static constexpr uint32_t period(type const options) noexcept
   {
      return options.period;
   }

   [[nodiscard]] static constexpr double sum_x(type const options) noexcept
   {
      return options.sumX;
   }
};

}

struct linear_regression_result final
{
   double intercept{std::numeric_limits<double>::signaling_NaN(),};
   double slope{std::numeric_limits<double>::signaling_NaN(),};
};

template<uint32_t indicator_period = lazy_indicator, typename y_values_allocator = std::allocator<double>>
class linear_regression final
{
private:
   using options = internals::linear_regression_options<indicator_period>;
   using options_traits = internals::indicator_traits<options>;

public:
   [[maybe_unused, nodiscard]] constexpr linear_regression() noexcept requires(lazy_indicator != indicator_period) :
      m_storage{}
   {
      static_assert(1u < period());
#if (not defined(NDEBUG))
      std::ranges::fill(m_storage.yValues, std::numeric_limits<double>::signaling_NaN());
#endif
   }

   linear_regression(linear_regression &&) = delete;
   linear_regression(linear_regression const &) = delete;

   [[maybe_unused, nodiscard]] constexpr explicit linear_regression(
      uint32_t const inPeriod
   ) requires(lazy_indicator == indicator_period) :
      linear_regression{inPeriod, y_values_allocator{},}
   {}

   [[nodiscard]] constexpr linear_regression(
      uint32_t const inPeriod,
      y_values_allocator const &allocator
   ) requires(lazy_indicator == indicator_period) :
      m_storage
      {
         .options = options
         {
            .period = inPeriod,
            .lookbackPeriod = internals::linear_regression_lookback_period(inPeriod),
            .sumX = internals::linear_regression_sum_x(inPeriod),
            .divisor = internals::linear_regression_divisor(inPeriod),
         },
         .yValues = std::vector<double, y_values_allocator>
         {
            inPeriod,
#if (not defined(NDEBUG))
            std::numeric_limits<double>::signaling_NaN(),
#endif
            allocator,
         },
      }
   {
      assert(1u < period());
   }

   linear_regression &operator = (linear_regression &&) = delete;
   linear_regression &operator = (linear_regression const &) = delete;

   [[maybe_unused, nodiscard]] constexpr linear_regression_result calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
#if (not defined(NDEBUG))
      assert(true == m_sequenceChecker.calc(inSequenceNumber));
#endif
      assert(true == std::isfinite(inValue));
      m_storage.yValues[inSequenceNumber % period()] = inValue;
      if (lookback_period() <= inSequenceNumber) [[likely]]
      {
         return do_calc(inSequenceNumber);
      }
      return linear_regression_result{};
   }

   [[nodiscard]] constexpr uint32_t lookback_period() const noexcept requires(lazy_indicator == indicator_period)
   {
      return options_traits::lookback_period(m_storage.options);
   }

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept requires(lazy_indicator != indicator_period)
   {
      return options_traits::lookback_period();
   }

   [[nodiscard]] constexpr uint32_t period() const noexcept requires(lazy_indicator == indicator_period)
   {
      return options_traits::period(m_storage.options);
   }

   [[nodiscard]] static constexpr uint32_t period() noexcept requires(lazy_indicator != indicator_period)
   {
      return options_traits::period();
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
#if (not defined(NDEBUG))
      std::ranges::fill(m_storage.yValues, std::numeric_limits<double>::signaling_NaN());
      m_sequenceChecker.reset();
#endif
   }

private:
   internals::linear_regression_storage<indicator_period, y_values_allocator> m_storage;
#if (not defined(NDEBUG))
   sequence_checker m_sequenceChecker{};
#endif

   [[nodiscard]] constexpr double divisor() const noexcept requires(lazy_indicator == indicator_period)
   {
      return options_traits::divisor(m_storage.options);
   }

   [[nodiscard]] static constexpr double divisor() noexcept requires(lazy_indicator != indicator_period)
   {
      return options_traits::divisor();
   }

   [[nodiscard]] constexpr linear_regression_result do_calc(uint64_t const inSequenceNumber) const noexcept
   {
      auto sumY{0e0,};
      auto sumXY{0e0,};
      for (auto const x : std::views::iota(0u, period()) | std::views::reverse)
      {
         auto const y{m_storage.yValues[(inSequenceNumber - x) % period()], };
         sumY += y;
         sumXY += x * y;
      }
      auto const slope{(period() * sumXY - sum_x() * sumY) / divisor(),};
      return linear_regression_result
      {
         .intercept = (sumY - slope * sum_x()) / period(),
         .slope = slope,
      };
   }

   [[nodiscard]] constexpr double sum_x() const noexcept requires(lazy_indicator == indicator_period)
   {
      return options_traits::sum_x(m_storage.options);
   }

   [[nodiscard]] static constexpr double sum_x() noexcept requires(lazy_indicator != indicator_period)
   {
      return options_traits::sum_x();
   }
};

}
