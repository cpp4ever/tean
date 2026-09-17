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
#include <memory> /// for std::allocator
#include <vector> /// for std::vector

namespace tean
{

namespace internals
{

template<uint32_t indicator_period>
struct sum_over_period_options final
{};

template<>
struct sum_over_period_options<lazy_indicator> final
{
   uint32_t const period;
   uint32_t const lookbackPeriod;
};

template<uint32_t indicator_period, typename values_allocator>
struct sum_over_period_storage final
{
   std::array<double, indicator_period> values{};
   double sum{0e0,};
};

template<typename values_allocator>
struct sum_over_period_storage<lazy_indicator, values_allocator> final
{
   sum_over_period_options<lazy_indicator> const options;
   std::vector<double, values_allocator> values;
   double sum{0e0,};
};

[[nodiscard]] constexpr uint32_t sum_over_period_lookback_period(uint32_t const inPeriod) noexcept
{
   return inPeriod - 1u;
}

template<uint32_t indicator_period>
struct indicator_traits<sum_over_period_options<indicator_period>> final
{
   using type = sum_over_period_options<indicator_period>;

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept
   {
      return sum_over_period_lookback_period(period());
   }

   [[nodiscard]] static constexpr uint32_t period() noexcept
   {
      return indicator_period;
   }
};

template<>
struct indicator_traits<sum_over_period_options<lazy_indicator>> final
{
   using type = sum_over_period_options<lazy_indicator>;

   [[nodiscard]] static constexpr uint32_t lookback_period(type const options) noexcept
   {
      return options.lookbackPeriod;
   }

   [[nodiscard]] static constexpr uint32_t period(type const options) noexcept
   {
      return options.period;
   }
};

}

template<uint32_t indicator_period = lazy_indicator, typename values_allocator = std::allocator<double>>
class sum_over_period final
{
private:
   using options = internals::sum_over_period_options<indicator_period>;
   using options_traits = internals::indicator_traits<options>;

public:
   [[maybe_unused, nodiscard]] constexpr explicit sum_over_period() noexcept requires(lazy_indicator != indicator_period)
   {
      static_assert(1u < period());
      std::ranges::fill(m_storage.values, 0e0);
   }

   sum_over_period(sum_over_period &&) = delete;
   sum_over_period(sum_over_period const &) = delete;

   [[maybe_unused, nodiscard]] constexpr explicit sum_over_period(
      uint32_t const inPeriod
   ) requires(lazy_indicator == indicator_period) :
      sum_over_period{inPeriod, values_allocator{},}
   {}

   [[nodiscard]] constexpr sum_over_period(
      uint32_t const inPeriod,
      values_allocator const &allocator
   ) requires(lazy_indicator == indicator_period) :
      m_storage
      {
         .options = options
         {
            .period = inPeriod,
            .lookbackPeriod = internals::sum_over_period_lookback_period(inPeriod),
         },
         .values = std::vector<double, values_allocator>{inPeriod, 0e0, allocator,},
      }
   {
      assert(1u < period());
   }

   sum_over_period &operator = (sum_over_period &&) = delete;
   sum_over_period &operator = (sum_over_period const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
#if (not defined(NDEBUG))
      assert(true == m_sequenceChecker.calc(inSequenceNumber));
#endif
      assert(true == std::isfinite(inValue));
      auto &prevValue{m_storage.values[inSequenceNumber % period()],};
      m_storage.sum += inValue - prevValue;
      prevValue = inValue;
      return m_storage.sum;
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

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
#if (not defined(NDEBUG))
      assert(true == m_sequenceChecker.pick(inSequenceNumber));
#endif
      assert(true == std::isfinite(inValue));
      return inValue - m_storage.values[inSequenceNumber % period()] + m_storage.sum;
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      std::ranges::fill(m_storage.values, 0e0);
      m_storage.sum = 0e0;
#if (not defined(NDEBUG))
      m_sequenceChecker.reset();
#endif
   }

private:
   internals::sum_over_period_storage<indicator_period, values_allocator> m_storage;
#if (not defined(NDEBUG))
   sequence_checker m_sequenceChecker{};
#endif
};

}
