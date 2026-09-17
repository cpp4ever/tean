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

template<uint32_t indicator_period>
struct relative_strength_index_options final
{};

template<>
struct relative_strength_index_options<lazy_indicator> final
{
   uint32_t const period;
};

template<uint32_t indicator_period>
struct relative_strength_index_storage final
{
   double smoothGain{0e0,};
   double smoothLoss{0e0,};
   double value{std::numeric_limits<double>::signaling_NaN(),};
};

template<>
struct relative_strength_index_storage<lazy_indicator> final
{
   relative_strength_index_options<lazy_indicator> const options;
   double smoothGain{0e0,};
   double smoothLoss{0e0,};
   double value{std::numeric_limits<double>::signaling_NaN(),};
};

[[nodiscard]] constexpr uint32_t relative_strength_index_lookback_period(uint32_t const inPeriod) noexcept
{
   return inPeriod;
}

template<uint32_t indicator_period>
struct indicator_traits<relative_strength_index_options<indicator_period>> final
{
   using type = relative_strength_index_options<indicator_period>;

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept
   {
      return relative_strength_index_lookback_period(period());
   }

   [[nodiscard]] static constexpr uint32_t period() noexcept
   {
      return indicator_period;
   }
};

template<>
struct indicator_traits<relative_strength_index_options<lazy_indicator>> final
{
   using type = relative_strength_index_options<lazy_indicator>;

   [[nodiscard]] static constexpr uint32_t lookback_period(type const options) noexcept
   {
      return period(options);
   }

   [[nodiscard]] static constexpr uint32_t period(type const options) noexcept
   {
      return options.period;
   }
};

}

template<uint32_t indicator_period = lazy_indicator>
class relative_strength_index final
{
private:
   using options = internals::relative_strength_index_options<indicator_period>;
   using options_traits = internals::indicator_traits<options>;

public:
   [[maybe_unused, nodiscard]] constexpr relative_strength_index() noexcept requires(lazy_indicator != indicator_period) :
      m_storage{}
   {
      static_assert(1u < indicator_period);
   }

   relative_strength_index(relative_strength_index &&) = delete;
   relative_strength_index(relative_strength_index const &) = delete;

   [[maybe_unused, nodiscard]] constexpr explicit relative_strength_index(
      uint32_t const inPeriod
   ) noexcept requires(lazy_indicator == indicator_period) :
      m_storage{.options = options{.period = inPeriod,},}
   {
      assert(1u < period());
   }

   relative_strength_index &operator = (relative_strength_index &&) = delete;
   relative_strength_index &operator = (relative_strength_index const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t inSequenceNumber, double inValue) noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
      m_prevSequenceNumber = inSequenceNumber;
#endif
      assert(true == std::isfinite(inValue));
      if (lookback_period() < inSequenceNumber) [[likely]]
      {
         return do_regular_calc(inValue);
      }
      return do_lookback_calc(inSequenceNumber, inValue);
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

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t inSequenceNumber, double inValue) const noexcept
   {
#if (not defined(NDEBUG))
      assert(((m_prevSequenceNumber + 1ull) == inSequenceNumber) || ((0ull == m_prevSequenceNumber) && (0ull == inSequenceNumber)));
#endif
      assert(true == std::isfinite(inValue));
      if (lookback_period() < inSequenceNumber) [[likely]]
      {
         return do_regular_pick(inValue);
      }
      if (lookback_period() == inSequenceNumber) [[unlikely]]
      {
         return do_lookback_pick(inValue);
      }
      return std::numeric_limits<double>::signaling_NaN();
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      m_storage.smoothGain = 0e0;
      m_storage.smoothLoss = 0e0;
      m_storage.value = std::numeric_limits<double>::signaling_NaN();
#if (not defined(NDEBUG))
      m_prevSequenceNumber = 0ull;
#endif
   }

private:
   internals::relative_strength_index_storage<indicator_period> m_storage;
#if (not defined(NDEBUG))
   uint64_t m_prevSequenceNumber{0ull,};
#endif

   [[nodiscard]] constexpr double do_lookback_calc(uint64_t inSequenceNumber, double inValue) noexcept
   {
      if (0ull < inSequenceNumber) [[likely]]
      {
         if (auto const delta{inValue - m_storage.value,}; 0e0 < delta)
         {
            m_storage.smoothGain += delta;
         }
         else
         {
            m_storage.smoothLoss -= delta;
         }
      }
      m_storage.value = inValue;
      if (lookback_period() == inSequenceNumber) [[unlikely]]
      {
         m_storage.smoothGain /= period();
         m_storage.smoothLoss /= period();
         auto const smoothTotal{m_storage.smoothGain + m_storage.smoothLoss,};
         return (0e0 == smoothTotal) ? 0e0 : (100e0 * (m_storage.smoothGain / smoothTotal));
      }
      return std::numeric_limits<double>::signaling_NaN();
   }

   [[nodiscard]] constexpr double do_lookback_pick(double inValue) const noexcept
   {
      double smoothGain;
      double smoothLoss;
      if (auto const delta{inValue - m_storage.value,}; 0e0 < delta)
      {
         smoothGain = (m_storage.smoothGain + delta) / period();
         smoothLoss = m_storage.smoothLoss / period();
      }
      else
      {
         smoothGain = m_storage.smoothGain / period();
         smoothLoss = (m_storage.smoothLoss - delta) / period();
      }
      auto const smoothTotal{smoothGain + smoothLoss,};
      return (0e0 == smoothTotal) ? 0e0 : (100e0 * (smoothGain / smoothTotal));
   }

   [[nodiscard]] constexpr double do_regular_calc(double inValue) noexcept
   {
      if (auto const delta{inValue - m_storage.value,}; 0e0 < delta)
      {
         m_storage.smoothGain += (delta - m_storage.smoothGain) / period();
         m_storage.smoothLoss -= m_storage.smoothLoss / period();
      }
      else
      {
         m_storage.smoothGain -= m_storage.smoothGain / period();
         m_storage.smoothLoss += (-delta - m_storage.smoothLoss) / period();
      }
      m_storage.value = inValue;
      auto const smoothTotal{m_storage.smoothGain + m_storage.smoothLoss,};
      return (0e0 == smoothTotal) ? 0e0 : (100e0 * (m_storage.smoothGain / smoothTotal));
   }

   [[nodiscard]] constexpr double do_regular_pick(double inValue) const noexcept
   {
      double smoothGain;
      double smoothLoss;
      if (auto const delta{inValue - m_storage.value,}; 0e0 < delta)
      {
         smoothGain = m_storage.smoothGain + (delta - m_storage.smoothGain) / period();
         smoothLoss = m_storage.smoothLoss - m_storage.smoothLoss / period();
      }
      else
      {
         smoothGain = m_storage.smoothGain - m_storage.smoothGain / period();
         smoothLoss = m_storage.smoothLoss + (-delta - m_storage.smoothLoss) / period();
      }
      auto const smoothTotal{smoothGain + smoothLoss,};
      return (0e0 == smoothTotal) ? 0e0 : (100e0 * (smoothGain / smoothTotal));
   }
};

}
