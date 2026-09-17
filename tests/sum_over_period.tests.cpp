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

#include "decimal.tests.hpp" /// for tean::tests::decimal, tean::tests::inverted_power_of_ten, tean::tests::power_of_ten
#include "tean.tests.hpp" /// for tean::tests::TeAn

#include "tean/sum_over_period.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleNear, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_DOUBLE_EQ, ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE
#include <indicators.h> /// for TI_OKAY, ti_sum, ti_sum_start
#include <ta_func.h> /// for TA_SUCCESS, TA_SUM, TA_SUM_Lookback

#include <algorithm> /// for std::ranges::fill
#include <cmath> /// for std::isfinite
#include <cstdint> /// for int64_t, uint32_t
#include <memory> /// for std::addressof, std::make_unique
#include <ranges> ///< for std::views::iota
#include <vector> /// for std::vector

namespace tean::tests
{

template<uint32_t test_period>
void test_sum_over_period_step(TeAn &fixture, decimal const testPriceStep)
{
   constexpr auto testLookbackPeriod{sum_over_period<test_period>::lookback_period(),};
   constexpr auto testIterationsNumber{test_period * 10u,};
   auto const testPrices{std::make_unique<double[]>(testLookbackPeriod + testIterationsNumber),};
   auto testValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
   {
      sum_over_period<test_period> testIndicator{};
      constexpr auto testPricePrecision{inverted_power_of_ten[12u],};
      double const testPriceStepValue{testPriceStep,};
      {
         auto testSum{0e0,};
         for (auto const testIteration : std::views::iota(0u, testLookbackPeriod))
         {
            auto const testPrice{testPriceStepValue * fixture.random_number(100u, 1000u),};
            auto const testPickValue{testIndicator.pick(testIteration, testPrice),};
            ASSERT_TRUE(std::isfinite(testPickValue));
            testSum += testPrice;
            ASSERT_DOUBLE_EQ(testPickValue, testSum);
            auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
            ASSERT_TRUE(std::isfinite(testCalcValue));
            ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            testPrices[testIteration] = testPrice;
         }
      }
      for (auto const testIteration : std::views::iota(0u, testIterationsNumber))
      {
         auto const testPrice{testPriceStepValue * fixture.random_number(100u, 1000u),};
         auto const testPickValue{testIndicator.pick(testLookbackPeriod + testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testPickValue));
         auto const testCalcValue{testIndicator.calc(testLookbackPeriod + testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testCalcValue));
         ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
         testPrices[testLookbackPeriod + testIteration] = testPrice;
         testValues[testIteration] = testing::DoubleNear(testCalcValue, testPricePrecision);
      }
      auto const testMatcher{testing::ElementsAreArray(testValues.get(), testIterationsNumber),};
      std::vector<double> expectedValues{};
      expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
      {
         ASSERT_EQ(TA_SUM_Lookback(static_cast<int>(test_period)), static_cast<int>(testLookbackPeriod));
         auto expectedFirstIndex{0,};
         auto expectedNumberOfElements{0,};
         ASSERT_EQ(TA_SUM(
            0,
            static_cast<int>(testLookbackPeriod + testIterationsNumber) - 1,
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
         {
            auto testSum{0e0,};
            for (auto const testIteration : std::views::iota(0u, testLookbackPeriod))
            {
               auto const testPrice{testPrices[testIteration],};
               auto const testPickValue{testIndicator.pick(testIteration, testPrice),};
               ASSERT_TRUE(std::isfinite(testPickValue));
               testSum += testPrice;
               ASSERT_DOUBLE_EQ(testPickValue, testSum);
               auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
               ASSERT_TRUE(std::isfinite(testCalcValue));
               ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            }
         }
         {
            auto const testPrice{testPrices[testLookbackPeriod],};
            auto const testPickValue{testIndicator.pick(testLookbackPeriod, testPrice),};
            ASSERT_TRUE(std::isfinite(testPickValue));
            auto const testCalcValue{testIndicator.calc(testLookbackPeriod, testPrice),};
            ASSERT_TRUE(std::isfinite(testCalcValue));
            ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            ASSERT_THAT(expectedValues[0u], testing::DoubleNear(testCalcValue, testPricePrecision));
         }
      }
      std::ranges::fill(expectedValues, std::numeric_limits<double>::signaling_NaN());
      {
         double *testInputs[]{testPrices.get(),};
         double const testOptions[]{test_period,};
         double *testOutputs[]{expectedValues.data(),};
         ASSERT_EQ(ti_sum_start(testOptions), static_cast<int>(testLookbackPeriod));
         ASSERT_EQ(TI_OKAY, ti_sum(static_cast<int>(testLookbackPeriod + testIterationsNumber), testInputs, testOptions, testOutputs));
         ASSERT_THAT(expectedValues, testMatcher);
      }
   }
   {
      sum_over_period<> testIndicator{test_period,};
      ASSERT_EQ(test_period, testIndicator.period());
      ASSERT_EQ(testLookbackPeriod, testIndicator.lookback_period());
      for (auto const testIteration : std::views::iota(0u, testLookbackPeriod + testIterationsNumber))
      {
         auto const testPrice{testPrices[testIteration],};
         auto const testPickValue{testIndicator.pick(testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testPickValue));
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testCalcValue));
         ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
         if (testLookbackPeriod <= testIteration)
         {
            ASSERT_THAT(testCalcValue, testValues[testIteration - testLookbackPeriod]);
         }
      }
      testIndicator.reset();
      for (auto const testIteration : std::views::iota(0u, testLookbackPeriod + 1u))
      {
         auto const testPrice{testPrices[testIteration],};
         auto const testPickValue{testIndicator.pick(testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testPickValue));
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testCalcValue));
         ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
         if (testLookbackPeriod == testIteration)
         {
            ASSERT_THAT(testCalcValue, testValues[testIteration - testLookbackPeriod]);
         }
      }
   }
}

template<uint32_t test_period>
void test_sum_over_period(TeAn &fixture, decimal const testPriceStep)
{
   test_sum_over_period_step<test_period>(fixture, testPriceStep);
   if constexpr (2u < test_period)
   {
      test_sum_over_period<test_period - 1u>(fixture, testPriceStep);
   }
}

TEST_F(TeAn, SumOverPeriod)
{
   constexpr auto testMaxPeriod{100u,};
   ASSERT_NO_FATAL_FAILURE(test_sum_over_period<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[0u]), .scale = 12,}));
   ASSERT_NO_FATAL_FAILURE(test_sum_over_period<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[6u]), .scale =  0,}));
}

}
