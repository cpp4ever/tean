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

#include "tean/accumulation_distribution_line.hpp" /// for tean::accumulation_distribution_line

#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint64_t

namespace tean
{

double accumulation_distribution_line::calc([[maybe_unused]] uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double const inVolume) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(true == std::isfinite(inHigh));
   assert(false == std::isnan(inHigh));
   assert(true == std::isfinite(inLow));
   assert(false == std::isnan(inLow));
   assert(true == std::isfinite(inClose));
   assert(false == std::isnan(inClose));
   assert(true == std::isfinite(inVolume));
   assert(false == std::isnan(inVolume));
   assert(inHigh >= inLow);
   assert(inHigh >= inClose);
   assert(inClose >= inLow);
   assert(0.0 <= inVolume);
   auto const body = inHigh - inLow;
   if (0.0 < body)
   {
      auto const value = inVolume * ((inClose - inLow) - (inHigh - inClose)) / body;
      assert(true == std::isfinite(value));
      assert(false == std::isnan(value));
      m_value += value;
   }
   return m_value;
}

double accumulation_distribution_line::pick([[maybe_unused]] uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose, double const inVolume) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
#endif
   assert(true == std::isfinite(inHigh));
   assert(false == std::isnan(inHigh));
   assert(true == std::isfinite(inLow));
   assert(false == std::isnan(inLow));
   assert(true == std::isfinite(inClose));
   assert(false == std::isnan(inClose));
   assert(true == std::isfinite(inVolume));
   assert(false == std::isnan(inVolume));
   assert(inHigh >= inLow);
   assert(inHigh >= inClose);
   assert(inClose >= inLow);
   assert(0.0 <= inVolume);
   auto const body = inHigh - inLow;
   if (0.0 < body)
   {
      auto const value = inVolume * ((inClose - inLow) - (inHigh - inClose)) / body;
      assert(true == std::isfinite(value));
      assert(false == std::isnan(value));
      return m_value + value;
   }
   return m_value;
}

}
