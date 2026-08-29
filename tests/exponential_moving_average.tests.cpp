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

#include "decimal.tests.hpp" /// for tean::tests::decimal, tean::tests::power_of_ten
#include "tean.tests.hpp" /// for tean::tests::TeAn

#include "tean/exponential_moving_average.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleEq, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_DOUBLE_EQ, ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE
#include <ta_func.h> /// for TA_EMA, TA_EMA_Lookback, TA_FUNC_UNST_EMA, TA_SetUnstablePeriod, TA_SUCCESS

#include <memory> /// for std::addressof, std::make_unique
#include <ranges> ///< for std::views::iota
#include <vector> /// for std::vector

namespace tean::tests
{

template<uint32_t test_period>
void test_exponential_moving_average_step(TeAn &fixture, decimal const testPriceStep)
{
   constexpr auto testLookbackPeriod{exponential_moving_average<test_period, test_period>::lookback_period,};
   constexpr auto testIterationsNumber{test_period * 10ui32,};
   auto const testPrices{std::make_unique<double[]>(testLookbackPeriod + testIterationsNumber),};
   auto const testValues{std::make_unique<testing::Matcher<double>[]>(testIterationsNumber),};
   {
      double const testPriceStepValue{testPriceStep,};
      exponential_moving_average<test_period, test_period> testIndicator{};
      for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(100ui32, 1000ui32),};
         auto const testPickValue{testIndicator.pick(testIteration, testPrice),};
         ASSERT_FALSE(std::isfinite(testPickValue));
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         ASSERT_FALSE(std::isfinite(testCalcValue));
         testPrices[testIteration] = testPrice;
      }
      for (auto const testIteration : std::views::iota(0ui32, testIterationsNumber))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(100ui32, 1000ui32),};
         auto const testPickValue{testIndicator.pick(testLookbackPeriod + testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testPickValue));
         auto const testCalcValue{testIndicator.calc(testLookbackPeriod + testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testCalcValue));
         ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
         testPrices[testLookbackPeriod + testIteration] = testPrice;
         testValues[testIteration] = testing::DoubleEq(testCalcValue);
      }
      auto const testMatcher{testing::ElementsAreArray(testValues.get(), testIterationsNumber),};
      std::vector<double> expectedValues{};
      expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
      {
         ASSERT_EQ(TA_SetUnstablePeriod(TA_FUNC_UNST_EMA, static_cast<int>(test_period)), TA_SUCCESS);
         ASSERT_EQ(TA_EMA_Lookback(static_cast<int>(test_period)), static_cast<int>(testLookbackPeriod));
         auto expectedFirstIndex{0i32,};
         auto expectedNumberOfElements{0i32,};
         ASSERT_EQ(TA_EMA(
            0i32,
            static_cast<int>(testLookbackPeriod + testIterationsNumber) - 1i32,
            testPrices.get(),
            static_cast<int>(test_period),
            std::addressof(expectedFirstIndex),
            std::addressof(expectedNumberOfElements),
            expectedValues.data()
         ), TA_SUCCESS);
         ASSERT_EQ(expectedFirstIndex, static_cast<int>(testLookbackPeriod));
         ASSERT_EQ(expectedNumberOfElements, static_cast<int>(testIterationsNumber));
         ASSERT_THAT(expectedValues, testMatcher);
         testIndicator.reset();
         for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod))
         {
            auto const testPrice{testPrices[testIteration],};
            auto const testPickValue{testIndicator.pick(testIteration, testPrice),};
            ASSERT_FALSE(std::isfinite(testPickValue));
            auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         {
            auto const testPrice{testPrices[testLookbackPeriod],};
            auto const testPickValue{testIndicator.pick(testLookbackPeriod, testPrice),};
            ASSERT_TRUE(std::isfinite(testPickValue));
            auto const testCalcValue{testIndicator.calc(testLookbackPeriod, testPrice),};
            ASSERT_TRUE(std::isfinite(testCalcValue));
            ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            ASSERT_DOUBLE_EQ(expectedValues[0ui32], testCalcValue);
         }
      }
   }
   {
      exponential_moving_average<> testIndicator{test_period, test_period,};
      ASSERT_EQ(test_period, testIndicator.period());
      ASSERT_EQ(testLookbackPeriod, testIndicator.lookback_period());
      for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod + testIterationsNumber))
      {
         auto const testPickValue{testIndicator.pick(testIteration, testPrices[testIteration]),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrices[testIteration]),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testPickValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         else
         {
            ASSERT_THAT(testPickValue, testValues[testIteration - testLookbackPeriod]);
            ASSERT_THAT(testCalcValue, testValues[testIteration - testLookbackPeriod]);
         }
      }
      testIndicator.reset();
      for (auto const testIteration : std::views::iota(0ui32, testLookbackPeriod + 1ui32))
      {
         auto const testPickValue{testIndicator.pick(testIteration, testPrices[testIteration]),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrices[testIteration]),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testPickValue));
            ASSERT_FALSE(std::isfinite(testCalcValue));
         }
         else
         {
            ASSERT_THAT(testPickValue, testValues[testIteration - testLookbackPeriod]);
            ASSERT_THAT(testCalcValue, testValues[testIteration - testLookbackPeriod]);
         }
      }
   }
}

template<uint32_t test_period>
void test_exponential_moving_average(TeAn &fixture, decimal const testPriceStep)
{
   test_exponential_moving_average_step<test_period>(fixture, testPriceStep);
   if constexpr (2ui32 < test_period)
   {
      test_exponential_moving_average<test_period - 1ui32>(fixture, testPriceStep);
   }
}

TEST_F(TeAn, ExponentialMovingAverage)
{
   constexpr auto testMaxPeriod{100ui32,};
   ASSERT_NO_FATAL_FAILURE(test_exponential_moving_average<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[0ui32]), .scale = 12ui8,}));
   ASSERT_NO_FATAL_FAILURE(test_exponential_moving_average<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[6ui32]), .scale = 00ui8,}));
}

}
