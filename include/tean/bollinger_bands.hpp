/*
   Part of the TeAn Project (https://github.com/cpp4ever/tean), under the MIT License
   SPDX-License-Identifier: MIT

   Copyright (c) 2024 Mikhail Smirnov

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

#include "tean/standard_deviation.hpp" /// for tean::standard_deviation

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits
#include <type_traits> /// for std::is_constructible_v
#include <utility> /// for std::forward

namespace tean
{

template<typename moving_average>
class [[nodiscard]] bollinger_bands final
{
public:
   struct [[maybe_unused, nodiscard]] result_type final
   {
      [[maybe_unused]] double upper = std::numeric_limits<double>::signaling_NaN();
      [[maybe_unused]] double middle = std::numeric_limits<double>::signaling_NaN();
      [[maybe_unused]] double lower = std::numeric_limits<double>::signaling_NaN();
   };

public:
   bollinger_bands() = delete;
   bollinger_bands(bollinger_bands &&) = delete;
   bollinger_bands(bollinger_bands const &) = delete;

   template<typename... types> requires(true == std::is_constructible_v<moving_average, uint32_t, types...>)
   [[maybe_unused, nodiscard]] bollinger_bands(uint32_t const inPeriod, double const inUpperBandMultiplier, double const inLowerBandMultiplier, types &&...inValues) :
      m_upperBandMultiplier(inUpperBandMultiplier),
      m_lowerBandMultiplier(inLowerBandMultiplier),
      m_standardDeviation(inPeriod),
      m_movingAverage(inPeriod, std::forward<types>(inValues)...)
   {
      assert(true == std::isfinite(m_upperBandMultiplier));
      assert(false == std::isnan(m_upperBandMultiplier));
      assert(true == std::isfinite(m_lowerBandMultiplier));
      assert(false == std::isnan(m_lowerBandMultiplier));
      assert(m_standardDeviation.period() == m_movingAverage.period());
      assert(m_standardDeviation.lookback_period() <= m_movingAverage.lookback_period());
   }

   bollinger_bands &operator = (bollinger_bands &&) = delete;
   bollinger_bands &operator = (bollinger_bands const &) = delete;

   [[maybe_unused, nodiscard]] result_type calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      return make_value(inSequenceNumber, m_standardDeviation.calc(inSequenceNumber, inValue), m_movingAverage.calc(inSequenceNumber, inValue));
   }

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_movingAverage.lookback_period();
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_movingAverage.period();
   }

   [[maybe_unused, nodiscard]] result_type pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
      return make_value(inSequenceNumber, m_standardDeviation.pick(inSequenceNumber, inValue), m_movingAverage.pick(inSequenceNumber, inValue));
   }

   [[maybe_unused]] void reset() noexcept
   {
      m_standardDeviation.reset();
      m_movingAverage.reset();
   }

private:
   double const m_upperBandMultiplier;
   double const m_lowerBandMultiplier;
   standard_deviation m_standardDeviation;
   moving_average m_movingAverage;

   [[maybe_unused, nodiscard]] result_type make_value(uint64_t const inSequenceNumber, double const inStandardDeviation, double const inMovingAverage) const noexcept
   {
      if (m_movingAverage.lookback_period() <= inSequenceNumber) [[likely]]
      {
         assert(true == std::isfinite(inStandardDeviation));
         assert(false == std::isnan(inStandardDeviation));
         assert(true == std::isfinite(inMovingAverage));
         assert(false == std::isnan(inMovingAverage));
         return result_type
         {
            .upper = inMovingAverage + (inStandardDeviation * m_upperBandMultiplier),
            .middle = inMovingAverage,
            .lower = inMovingAverage - (inStandardDeviation * m_lowerBandMultiplier),
         };
      }
      assert(true == std::isnan(inMovingAverage));
      return result_type{};
   }
};

}
