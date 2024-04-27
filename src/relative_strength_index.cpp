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

#include "tean/relative_strength_index.hpp" /// for tean::relative_strength_index

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint32_t, uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

relative_strength_index::relative_strength_index(uint32_t const inPeriod) noexcept :
   m_period(inPeriod),
#if (not defined(NDEBUG))
   m_prevSequenceNumber(0),
#endif
   m_smoothGain(0.0),
   m_smoothLoss(0.0),
   m_value(std::numeric_limits<double>::quiet_NaN())
{
   assert(1 < period());
}

double relative_strength_index::calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   if (lookback_period() < inSequenceNumber) [[likely]]
   {
      return do_regular_calc(inValue);
   }
   return do_lookback_calc(inSequenceNumber, inValue);
}

double relative_strength_index::pick(uint64_t const inSequenceNumber, double const inValue) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inValue));
   assert(false == std::isnan(inValue));
   if (lookback_period() < inSequenceNumber) [[likely]]
   {
      return do_regular_pick(inValue);
   }
   if (lookback_period() == inSequenceNumber) [[unlikely]]
   {
      return do_lookback_pick(inValue);
   }
   return std::numeric_limits<double>::quiet_NaN();
}

void relative_strength_index::reset() noexcept
{
#if (not defined(NDEBUG))
   m_prevSequenceNumber = 0;
#endif
   m_smoothGain = 0.0;
   m_smoothLoss = 0.0;
   m_value = std::numeric_limits<double>::quiet_NaN();
}

double relative_strength_index::do_lookback_calc(uint64_t const inSequenceNumber, double const inValue) noexcept
{
   if (0 < inSequenceNumber) [[likely]]
   {
      auto const delta = inValue - m_value;
      if (0 < delta)
      {
         m_smoothGain += delta;
      }
      else
      {
         m_smoothLoss -= delta;
      }
   }
   m_value = inValue;
   if (lookback_period() == inSequenceNumber) [[unlikely]]
   {
      m_smoothGain /= static_cast<double>(period());
      m_smoothLoss /= static_cast<double>(period());
      auto const smoothTotal = m_smoothGain + m_smoothLoss;
      return (0.0 == smoothTotal) ? 0.0 : (100.0 * (m_smoothGain / smoothTotal));
   }
   return std::numeric_limits<double>::quiet_NaN();
}

double relative_strength_index::do_lookback_pick(double const inValue) const noexcept
{
   auto const delta = inValue - m_value;
   double smoothGain;
   double smoothLoss;
   if (0 < delta)
   {
      smoothGain = (m_smoothGain + delta) / static_cast<double>(period());
      smoothLoss = m_smoothLoss / static_cast<double>(period());
   }
   else
   {
      smoothGain = m_smoothGain / static_cast<double>(period());
      smoothLoss = (m_smoothLoss - delta) / static_cast<double>(period());
   }
   auto const smoothTotal = smoothGain + smoothLoss;
   return (0.0 == smoothTotal) ? 0.0 : (100.0 * (smoothGain / smoothTotal));
}

double relative_strength_index::do_regular_calc(double const inValue) noexcept
{
   auto const delta = inValue - m_value;
   if (0 < delta)
   {
      m_smoothGain += (delta - m_smoothGain) / static_cast<double>(period());
      m_smoothLoss -= m_smoothLoss / static_cast<double>(period());
   }
   else
   {
      m_smoothGain -= m_smoothGain / static_cast<double>(period());
      m_smoothLoss += (-delta - m_smoothLoss) / static_cast<double>(period());
   }
   m_value = inValue;
   auto const smoothTotal = m_smoothGain + m_smoothLoss;
   return (0.0 == smoothTotal) ? 0.0 : (100.0 * (m_smoothGain / smoothTotal));
}

double relative_strength_index::do_regular_pick(double const inValue) const noexcept
{
   auto const delta = inValue - m_value;
   double smoothGain;
   double smoothLoss;
   if (0 < delta)
   {
      smoothGain = m_smoothGain + (delta - m_smoothGain) / static_cast<double>(period());
      smoothLoss = m_smoothLoss - m_smoothLoss / static_cast<double>(period());
   }
   else
   {
      smoothGain = m_smoothGain - m_smoothGain / static_cast<double>(period());
      smoothLoss = m_smoothLoss + (-delta - m_smoothLoss) / static_cast<double>(period());
   }
   auto const smoothTotal = smoothGain + smoothLoss;
   return (0.0 == smoothTotal) ? 0.0 : (100.0 * (smoothGain / smoothTotal));
}

}
