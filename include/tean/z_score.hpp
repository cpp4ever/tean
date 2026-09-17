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

#include "tean/indicator_traits.hpp" ///< for tean::lazy_indicator
#include "tean/variance.hpp" ///< for tean::variance

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::sqrt
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

template<uint32_t indicator_period = lazy_indicator, typename values_allocator = std::allocator<double>>
class z_score final
{
public:
   [[maybe_unused, nodiscard]] constexpr z_score() noexcept requires(lazy_indicator != indicator_period) :
      m_variance{}
   {}

   z_score(z_score &&) = delete;
   z_score(z_score const &) = delete;

   [[nodiscard]] constexpr explicit z_score(uint32_t const inPeriod) requires(lazy_indicator == indicator_period) :
      m_variance{inPeriod,}
   {}

   [[maybe_unused, nodiscard]] constexpr z_score(
      uint32_t const inPeriod,
      values_allocator const &allocator
   ) requires(lazy_indicator == indicator_period) :
      m_variance{inPeriod, allocator,}
   {}

   z_score &operator = (z_score &&) = delete;
   z_score &operator = (z_score const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      auto mean{0e0};
      auto const variance{m_variance.calc(inSequenceNumber, inValue, mean),};
      return variance_to_z_score(inSequenceNumber, inValue, mean, variance);
   }

   [[nodiscard]] constexpr uint32_t lookback_period() const noexcept requires(lazy_indicator == indicator_period)
   {
      return m_variance.lookback_period();
   }

   [[nodiscard]] static constexpr uint32_t lookback_period() noexcept requires(lazy_indicator != indicator_period)
   {
      return variance<indicator_period>::lookback_period();
   }

   [[maybe_unused, nodiscard]] constexpr uint32_t period() const noexcept requires(lazy_indicator == indicator_period)
   {
      return m_variance.period();
   }

   [[maybe_unused, nodiscard]] static constexpr uint32_t period() noexcept requires(lazy_indicator != indicator_period)
   {
      return variance<indicator_period>::period();
   }

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
      auto mean{0e0};
      auto const variance{m_variance.pick(inSequenceNumber, inValue, mean),};
      return variance_to_z_score(inSequenceNumber, inValue, mean, variance);
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      m_variance.reset();
   }

private:
   variance<indicator_period> m_variance{};

   [[nodiscard]] constexpr double variance_to_z_score(
      uint64_t const inSequenceNumber,
      double const inValue,
      double const inMean,
      double const inVariance
   ) const noexcept
   {
      if (lookback_period() <= inSequenceNumber) [[likely]]
      {
         assert(true == std::isfinite(inVariance));
         return (0e0 >= inVariance) ? 0e0 : ((inValue - inMean) / std::sqrt(inVariance));
      }
      assert(false == std::isfinite(inVariance));
      return std::numeric_limits<double>::signaling_NaN();
   }
};

}
