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

#include <gtest/gtest.h> /// for testing::Test

#include <chrono> /// for std::chrono::system_clock
#include <random> /// for std::mt19937_64, std::uniform_int_distribution, std::uniform_real_distribution
#include <type_traits> /// for std::is_floating_point_v, std::is_integral_v

namespace tean::tests
{

class [[nodiscard]] TeAn : public testing::Test
{
private: using super = testing::Test;

public:
   [[maybe_unused, nodiscard]] TeAn() :
      m_randomEngine()
   {}

   TeAn(TeAn &&) = delete;
   TeAn(TeAn const &) = delete;

   TeAn &operator = (TeAn &&) = delete;
   TeAn &operator = (TeAn const &) = delete;

   [[maybe_unused, nodiscard]] bool random_bool()
   {
      return 1 == random_number<int>(0, 1);
   }

   template<typename type> requires(true == std::is_integral_v<type>)
   [[maybe_unused, nodiscard]] type random_number(type const lowerBound, type const upperBound)
   {
      return std::uniform_int_distribution<type>{lowerBound, upperBound}(m_randomEngine);
   }

   template<typename type> requires(true == std::is_floating_point_v<type>)
   [[maybe_unused, nodiscard]] type random_number(type const lowerBound, type const upperBound)
   {
      return std::uniform_real_distribution<type>{lowerBound, upperBound}(m_randomEngine);
   }

protected:
   void SetUp() override
   {
      m_randomEngine.seed(std::chrono::system_clock::now().time_since_epoch().count());
   }

private:
   std::mt19937_64 m_randomEngine;
};

}
