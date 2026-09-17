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
#include <vector> /// for std::vector

namespace tean
{

namespace internals
{

template<uint32_t indicator_period>
struct variance_options final
{};

template<>
struct variance_options<lazy_indicator> final
{
   uint32_t const period;
   uint32_t const lookbackPeriod;
};

template<uint32_t indicator_period, typename y_values_allocator>
struct variance_storage final
{
   double sum{0e0,};
   double sumOfSquares{0e0,};
   std::array<double, indicator_period> values{};
};

template<typename y_values_allocator>
struct variance_storage<lazy_indicator, y_values_allocator> final
{
   variance_options<lazy_indicator> const options;
   double sum{0e0,};
   double sumOfSquares{0e0,};
   std::vector<double, y_values_allocator> values;
};

[[nodiscard]] constexpr uint32_t variance_lookback_period(uint32_t const inPeriod) noexcept
{
   return inPeriod - 1u;
}

template<uint32_t indicator_period>
struct indicator_traits<variance_options<indicator_period>> final
{
   using type = variance_options<indicator_period>;

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept
   {
      return variance_lookback_period(period());
   }

   [[nodiscard]] static constexpr uint32_t period() noexcept
   {
      return indicator_period;
   }
};

template<>
struct indicator_traits<variance_options<lazy_indicator>> final
{
   using type = variance_options<lazy_indicator>;

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
class variance final
{
private:
   using options = internals::variance_options<indicator_period>;
   using options_traits = internals::indicator_traits<options>;

public:
   [[maybe_unused, nodiscard]] constexpr variance() noexcept requires(lazy_indicator != indicator_period) :
      m_storage{}
   {
      static_assert(1u < period());
#if (not defined(NDEBUG))
      std::ranges::fill(m_storage.values, std::numeric_limits<double>::signaling_NaN());
#endif
   }

   variance(variance &&) = delete;
   variance(variance const &) = delete;

   [[nodiscard]] constexpr explicit variance(uint32_t const inPeriod) requires(lazy_indicator == indicator_period) :
      variance{inPeriod, values_allocator{},}
   {}

   [[maybe_unused, nodiscard]] constexpr variance(
      uint32_t const inPeriod,
      values_allocator const &allocator
   ) requires(lazy_indicator == indicator_period) :
      m_storage
      {
         .options = options
         {
            .period = inPeriod,
            .lookbackPeriod = internals::variance_lookback_period(inPeriod),
         },
         .values = std::vector<double, values_allocator>
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

   variance &operator = (variance &&) = delete;
   variance &operator = (variance const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      double mean{};
      return calc(inSequenceNumber, inValue, mean);
   }

   [[nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
   {
#if (not defined(NDEBUG))
      assert(true == m_sequenceChecker.calc(inSequenceNumber));
#endif
      assert(true == std::isfinite(inValue));
      if (lookback_period() <= inSequenceNumber) [[likely]]
      {
         return do_regular_calc(inSequenceNumber, inValue, outMean);
      }
      do_lookback_calc(inSequenceNumber, inValue);
      outMean = std::numeric_limits<double>::signaling_NaN();
      return std::numeric_limits<double>::signaling_NaN();
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
      double mean{};
      return pick(inSequenceNumber, inValue, mean);
   }

   [[nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue, double &outMean) const noexcept
   {
#if (not defined(NDEBUG))
      assert(true == m_sequenceChecker.pick(inSequenceNumber));
#endif
      assert(true == std::isfinite(inValue));
      if (lookback_period() <= inSequenceNumber) [[likely]]
      {
         outMean = (m_storage.sum + inValue) / period();
         auto const meanOfSquares{(m_storage.sumOfSquares + inValue * inValue) / period(),};
         return meanOfSquares - outMean * outMean;
      }
      outMean = std::numeric_limits<double>::signaling_NaN();
      return std::numeric_limits<double>::signaling_NaN();
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      m_storage.sum = 0e0;
      m_storage.sumOfSquares = 0e0;
#if (not defined(NDEBUG))
      std::ranges::fill(m_storage.values, std::numeric_limits<double>::signaling_NaN());
      m_sequenceChecker.reset();
#endif
   }

private:
   internals::variance_storage<indicator_period, values_allocator> m_storage;
#if (not defined(NDEBUG))
   sequence_checker m_sequenceChecker{};
#endif

   constexpr void do_lookback_calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      m_storage.sum += inValue;
      m_storage.sumOfSquares += inValue * inValue;
      m_storage.values[inSequenceNumber % lookback_period()] = inValue;
   }

   [[nodiscard]] constexpr double do_regular_calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
   {
      m_storage.sum += inValue;
      m_storage.sumOfSquares += inValue * inValue;
      outMean = m_storage.sum / period();
      auto const meanOfSquares{m_storage.sumOfSquares / period(),};
      auto &prevValue{m_storage.values[inSequenceNumber % lookback_period()],};
      m_storage.sum -= prevValue;
      m_storage.sumOfSquares -= prevValue * prevValue;
      prevValue = inValue;
      return meanOfSquares - outMean * outMean;
   }
};

}
