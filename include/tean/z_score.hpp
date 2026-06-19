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

#include "tean/variance.hpp" ///< for tean::variance

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::sqrt
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

template<uint32_t period = static_cast<uint32_t>(-1)>
class z_score;

template<uint32_t period>
class [[maybe_unused]] z_score final
{
public:
   static constexpr inline auto lookback_period{variance<period>::lookback_period,};

   [[maybe_unused, nodiscard]] constexpr z_score() noexcept = default;
   z_score(z_score &&) = delete;
   z_score(z_score const &) = delete;

   z_score &operator = (z_score &&) = delete;
   z_score &operator = (z_score const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      double mean{};
      auto const variance{m_variance.calc(inSequenceNumber, inValue, mean),};
      return variance_to_z_score(inSequenceNumber, inValue, mean, variance);
   }

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
      double mean{};
      auto const variance{m_variance.pick(inSequenceNumber, inValue, mean),};
      return variance_to_z_score(inSequenceNumber, inValue, mean, variance);
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      m_variance.reset();
   }

private:
   variance<period> m_variance;

   [[nodiscard]] constexpr double variance_to_z_score(
      uint64_t const inSequenceNumber,
      double const inValue,
      double const inMean,
      double const inVariance
   ) const noexcept
   {
      if (lookback_period <= inSequenceNumber) [[likely]]
      {
         assert(true == std::isfinite(inVariance));
         return (0 >= inVariance) ? 0.0 : ((inValue - inMean) / std::sqrt(inVariance));
      }
      assert(false == std::isfinite(inVariance));
      return std::numeric_limits<double>::signaling_NaN();
   }
};

template<>
class [[maybe_unused]] z_score<static_cast<uint32_t>(-1)> final
{
public:
   z_score() = delete;
   z_score(z_score &&) = delete;
   z_score(z_score const &) = delete;

   [[maybe_unused, nodiscard]] explicit z_score(uint32_t const inPeriod) noexcept :
      m_variance(inPeriod)
   {}

   z_score &operator = (z_score &&) = delete;
   z_score &operator = (z_score const &) = delete;

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      double mean{};
      auto const variance{m_variance.calc(inSequenceNumber, inValue, mean),};
      return variance_to_z_score(inSequenceNumber, inValue, mean, variance);
   }

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_variance.lookback_period();
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_variance.period();
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
      double mean{};
      auto const variance{m_variance.pick(inSequenceNumber, inValue, mean),};
      return variance_to_z_score(inSequenceNumber, inValue, mean, variance);
   }

   [[maybe_unused]] void reset() noexcept
   {
      m_variance.reset();
   }

private:
   variance<> m_variance;

   [[nodiscard]] double variance_to_z_score(
      uint64_t const inSequenceNumber,
      double const inValue,
      double const inMean,
      double const inVariance
   ) const noexcept
   {
      if (lookback_period() <= inSequenceNumber) [[likely]]
      {
         assert(true == std::isfinite(inVariance));
         return (0 >= inVariance) ? 0.0 : ((inValue - inMean) / std::sqrt(inVariance));
      }
      assert(false == std::isfinite(inVariance));
      return std::numeric_limits<double>::signaling_NaN();
   }
};

}
