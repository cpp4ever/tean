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

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

namespace internals
{

template<uint32_t indicator_period, uint32_t indicator_untrusted_period>
struct exponential_moving_average_options final
{
   double const smoothingFactor;
};

template<>
struct exponential_moving_average_options<lazy_indicator, lazy_indicator> final
{
   uint32_t const period;
   uint32_t const lookbackPeriod;
   double const smoothingFactor;
};

[[nodiscard]] constexpr uint32_t exponential_moving_average_lookback_period(uint32_t const inPeriod, uint32_t const inUntrustedPeriod) noexcept
{
   return inUntrustedPeriod + inPeriod - 1u;
}

template<uint32_t indicator_period, uint32_t indicator_untrusted_period>
struct indicator_traits<exponential_moving_average_options<indicator_period, indicator_untrusted_period>> final
{
   using type = exponential_moving_average_options<indicator_period, indicator_untrusted_period>;

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept
   {
      return exponential_moving_average_lookback_period(period(), untrusted_period());
   }

   [[nodiscard]] static constexpr uint32_t period() noexcept
   {
      return indicator_period;
   }

   [[nodiscard]] static constexpr double smoothing_factor(type const options) noexcept
   {
      return options.smoothingFactor;
   }

   [[nodiscard]] static constexpr uint32_t untrusted_period() noexcept
   {
      return indicator_untrusted_period;
   }
};

template<>
struct indicator_traits<exponential_moving_average_options<lazy_indicator, lazy_indicator>> final
{
   using type = exponential_moving_average_options<lazy_indicator, lazy_indicator>;

   [[nodiscard]] static constexpr uint32_t lookback_period(type const options) noexcept
   {
      return options.lookbackPeriod;
   }

   [[nodiscard]] static constexpr uint32_t period(type const options) noexcept
   {
      return options.period;
   }

   [[nodiscard]] static constexpr double smoothing_factor(type const options) noexcept
   {
      return options.smoothingFactor;
   }
};

[[nodiscard]] constexpr double exponential_moving_average_smoothing_factor(uint32_t const inPeriod, double const inSmoothing) noexcept
{
   return inSmoothing / (inPeriod + 1u);
}

}

template<uint32_t indicator_period = lazy_indicator, uint32_t indicator_untrusted_period = indicator_period>
class exponential_moving_average final
{
   static_assert(
      false
      || ((lazy_indicator == indicator_period) && (lazy_indicator == indicator_untrusted_period))
      || ((lazy_indicator != indicator_period) && (lazy_indicator != indicator_untrusted_period))
   );

private:
   using options = internals::exponential_moving_average_options<indicator_period, indicator_untrusted_period>;
   using options_traits = internals::indicator_traits<options>;

public:
   exponential_moving_average(exponential_moving_average &&) = delete;
   exponential_moving_average(exponential_moving_average const &) = delete;

   [[maybe_unused, nodiscard]] constexpr exponential_moving_average(
      uint32_t const inPeriod,
      uint32_t const inUntrustedPeriod,
      double inSmoothing = 2e0
   ) noexcept requires(lazy_indicator == indicator_period) :
      m_options
      {
         .period = inPeriod,
         .lookbackPeriod = internals::exponential_moving_average_lookback_period(inPeriod, inUntrustedPeriod),
         .smoothingFactor = internals::exponential_moving_average_smoothing_factor(inPeriod, inSmoothing),
      }
   {
      assert(1u < period());
      assert(true == std::isfinite(smoothing_factor()));
   }

   [[maybe_unused, nodiscard]] constexpr explicit exponential_moving_average(
      double const inSmoothing = 2e0
   ) noexcept requires(lazy_indicator != indicator_period) :
      m_options{.smoothingFactor = internals::exponential_moving_average_smoothing_factor(indicator_period, inSmoothing),}
   {
      static_assert(1u < period());
      assert(true == std::isfinite(smoothing_factor()));
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
      if (period() <= inSequenceNumber) [[likely]]
      {
         m_value += smoothing_factor() * (inValue - m_value);
         if (lookback_period() <= inSequenceNumber) [[likely]]
         {
            return m_value;
         }
      }
      else
      {
         m_value += inValue;
         if (period() == (inSequenceNumber + 1ull))
         {
            m_value /= period();
            if (lookback_period() == inSequenceNumber)
            {
               return m_value;
            }
         }
      }
      return std::numeric_limits<double>::signaling_NaN();
   }

   [[nodiscard]] constexpr uint32_t lookback_period() const noexcept requires(lazy_indicator == indicator_period)
   {
      return options_traits::lookback_period(m_options);
   }

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept requires(lazy_indicator != indicator_period)
   {
      return options_traits::lookback_period();
   }

   [[nodiscard]] constexpr uint32_t period() const noexcept requires(lazy_indicator == indicator_period)
   {
      return options_traits::period(m_options);
   }

   [[nodiscard]] static constexpr uint32_t period() noexcept requires(lazy_indicator != indicator_period)
   {
      return options_traits::period();
   }

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
#endif
      assert(true == std::isfinite(inValue));
      if ((period() <= inSequenceNumber) && (lookback_period() <= inSequenceNumber)) [[likely]]
      {
         return m_value + smoothing_factor() * (inValue - m_value);
      }
      if ((period() == (inSequenceNumber + 1ull)) && (lookback_period() == inSequenceNumber))
      {
         return (m_value + inValue) / period();
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

   [[nodiscard]] constexpr double smoothing_factor() const noexcept
   {
      return options_traits::smoothing_factor(m_options);
   }

private:
   options const m_options;
   double m_value{0e0,};
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0ull,};
#endif
};

}
