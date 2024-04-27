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

#include "tean/true_range.hpp" /// for tean::true_range

#include <algorithm> /// for std::max
#include <cassert> /// for assert
#include <cmath> /// for std::isfinite, std::isnan
#include <cstdint> /// for uint64_t
#include <cstdlib> /// for std::abs
#include <limits> /// for std::numeric_limits

namespace tean
{

double true_range::calc(uint64_t const inSequenceNumber, double const inHigh, double const inLow, double const inClose) noexcept
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
   assert(inHigh >= inLow);
   assert(inHigh >= inClose);
   assert(inClose >= inLow);
   auto const result = do_pick(inSequenceNumber, inHigh, inLow);
   m_close = inClose;
   return result;
}

double true_range::pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow, [[maybe_unused]] double const inClose) const noexcept
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
   assert(inHigh >= inLow);
   assert(inHigh >= inClose);
   assert(inClose >= inLow);
   return do_pick(inSequenceNumber, inHigh, inLow);
}

double true_range::do_pick(uint64_t const inSequenceNumber, double const inHigh, double const inLow) const noexcept
{
   if (0 < inSequenceNumber) [[likely]]
   {
      return std::max(
         {
            inHigh - inLow,
            std::abs(inHigh - m_close),
            std::abs(inLow - m_close),
         }
      );
   }
   return std::numeric_limits<double>::quiet_NaN();
}

}
