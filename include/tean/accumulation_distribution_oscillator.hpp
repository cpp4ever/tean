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

#include "tean/accumulation_distribution_line.hpp" /// for tean::accumulation_distribution_line

#include <cstdint> /// for uint32_t, uint64_t

namespace tean
{

class [[nodiscard]] accumulation_distribution_oscillator final
{
public:
   accumulation_distribution_oscillator() = delete;
   accumulation_distribution_oscillator(accumulation_distribution_oscillator &&) = delete;
   accumulation_distribution_oscillator(accumulation_distribution_oscillator const &) = delete;
   [[nodiscard]] accumulation_distribution_oscillator(uint32_t inFastPeriod, uint32_t inSlowPeriod, uint32_t inUntrustedPeriod) noexcept;

   accumulation_distribution_oscillator &operator = (accumulation_distribution_oscillator &&) = delete;
   accumulation_distribution_oscillator &operator = (accumulation_distribution_oscillator const &) = delete;

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double const inVolume) noexcept
   {
      return do_calc(inSequenceNumber, m_accumulationDistributionLine.calc(inSequenceNumber, inHigh, inLow, inClose, inVolume));
   }

   [[maybe_unused, nodiscard]] double calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double const inVolume, double &outAccumulationDistribution) noexcept
   {
      outAccumulationDistribution = m_accumulationDistributionLine.calc(inSequenceNumber, inHigh, inLow, inClose, inVolume);
      return do_calc(inSequenceNumber, outAccumulationDistribution);
   }

   [[maybe_unused, nodiscard]] uint32_t lookback_period() const noexcept
   {
      return m_lookbackPeriod;
   }

   [[maybe_unused, nodiscard]] uint32_t period() const noexcept
   {
      return m_period;
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double const inVolume) const noexcept
   {
      return do_pick(inSequenceNumber, m_accumulationDistributionLine.pick(inSequenceNumber, inHigh, inLow, inClose, inVolume));
   }

   [[maybe_unused, nodiscard]] double pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double const inVolume, double &outAccumulationDistribution) const noexcept
   {
      outAccumulationDistribution = m_accumulationDistributionLine.pick(inSequenceNumber, inHigh, inLow, inClose, inVolume);
      return do_pick(inSequenceNumber, outAccumulationDistribution);
   }

   [[maybe_unused]] void reset() noexcept
   {
      m_accumulationDistributionLine.reset();
      m_fastMovingAverage = 0.0;
      m_slowMovingAverage = 0.0;
   }

private:
   uint32_t const m_period;
   uint32_t const m_lookbackPeriod;
   double const m_fastSmoothingFactor;
   double const m_slowSmoothingFactor;
   accumulation_distribution_line m_accumulationDistributionLine;
   double m_fastMovingAverage;
   double m_slowMovingAverage;

   [[nodiscard]] double do_calc(uint64_t inSequenceNumber, double inAccumulationDistribution) noexcept;

   [[nodiscard]] double do_pick(uint64_t inSequenceNumber, double inAccumulationDistribution) const noexcept;
};

}
