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

#include "tean/ease_of_movement.hpp" /// for tean::ease_of_movement

#include <cassert> /// for assert
#include <cstdint> /// for uint64_t
#include <limits> /// for std::numeric_limits

namespace tean
{

double ease_of_movement::calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inVolume) noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
   m_prevSequenceNumber = inSequenceNumber;
#endif
   assert(inHigh >= inLow);
   assert(0.0 <= inVolume);
   auto emv = std::numeric_limits<double>::quiet_NaN();
   auto const mean = (inHigh + inLow) * 0.5;
   if (0 < inSequenceNumber) [[likely]]
   {
      auto const boxRatio = inVolume / 10000.0 / (inHigh - inLow);
      emv = (mean - m_lastMean) / boxRatio;
   }
   m_lastMean = mean;
   return emv;
}

double ease_of_movement::pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inVolume) const noexcept
{
#if (not defined(NDEBUG))
   assert(((m_prevSequenceNumber + 1) == inSequenceNumber) || ((0 == m_prevSequenceNumber) && (0 == inSequenceNumber)));
#endif
   assert(inHigh >= inLow);
   assert(0.0 <= inVolume);
   if (0 < inSequenceNumber) [[likely]]
   {
      auto const mean = (inHigh + inLow) * 0.5;
      auto const boxRatio = inVolume / 10000.0 / (inHigh - inLow);
      return (mean - m_lastMean) / boxRatio;
   }
   return std::numeric_limits<double>::quiet_NaN();
}

}
