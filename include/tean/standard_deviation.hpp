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

#include "tean/variance.hpp" /// for tean::variance

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::sqrt
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

template<uint32_t period = static_cast<uint32_t>(-1)>
class standard_deviation;

template<uint32_t period>
class [[maybe_unused]] standard_deviation final
{
public:
   static constexpr inline auto lookback_period{variance<period>::lookback_period,};

   [[maybe_unused, nodiscard]] constexpr standard_deviation() noexcept = default;
   standard_deviation(standard_deviation &&) = delete;
   standard_deviation(standard_deviation const &) = delete;

   standard_deviation &operator = (standard_deviation &&) = delete;
   standard_deviation &operator = (standard_deviation const &) = delete;

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.calc(inSequenceNumber, inValue));
   }

   [[maybe_unused, nodiscard]] constexpr double calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.calc(inSequenceNumber, inValue, outMean));
   }

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.pick(inSequenceNumber, inValue));
   }

   [[maybe_unused, nodiscard]] constexpr double pick(uint64_t const inSequenceNumber, double const inValue, double &outMean) const noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.pick(inSequenceNumber, inValue, outMean));
   }

   [[maybe_unused]] constexpr void reset() noexcept
   {
      m_variance.reset();
   }

private:
   variance<period> m_variance{};

   [[maybe_unused, nodiscard]] constexpr double variance_to_standard_deviation(uint64_t const inSequenceNumber, double const inVariance) const noexcept
   {
      if (lookback_period <= inSequenceNumber) [[likely]]
      {
         assert(true == std::isfinite(inVariance));
         return (0 >= inVariance) ? 0.0 : std::sqrt(inVariance);
      }
      assert(false == std::isfinite(inVariance));
      return std::numeric_limits<double>::signaling_NaN();
   }
};

template<>
class [[maybe_unused]] standard_deviation<static_cast<uint32_t>(-1)> final
{
public:
   standard_deviation() = delete;
   standard_deviation(standard_deviation &&) = delete;
   standard_deviation(standard_deviation const &) = delete;

   [[maybe_unused, nodiscard]] explicit standard_deviation(uint32_t const inPeriod) noexcept :
      m_variance(inPeriod)
   {}

   standard_deviation &operator = (standard_deviation &&) = delete;
   standard_deviation &operator = (standard_deviation const &) = delete;

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inValue) noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.calc(inSequenceNumber, inValue));
   }

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inValue, double &outMean) noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.calc(inSequenceNumber, inValue, outMean));
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
      return variance_to_standard_deviation(inSequenceNumber, m_variance.pick(inSequenceNumber, inValue));
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inValue, double &outMean) const noexcept
   {
      return variance_to_standard_deviation(inSequenceNumber, m_variance.pick(inSequenceNumber, inValue, outMean));
   }

   [[maybe_unused]] void reset() noexcept
   {
      m_variance.reset();
   }

private:
   variance<> m_variance;

   [[nodiscard]] double variance_to_standard_deviation(uint64_t inSequenceNumber, double inVariance) const noexcept;
};

}
