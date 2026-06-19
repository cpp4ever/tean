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

#include "tean/linear_regression.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleNear, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE
#include <ta_func.h> /// for TA_LINEARREG, TA_LINEARREG_INTERCEPT, TA_LINEARREG_INTERCEPT_Lookback, TA_LINEARREG_Lookback, TA_LINEARREG_SLOPE, TA_LINEARREG_SLOPE_Lookback, TA_SUCCESS, TA_TSF, TA_TSF_Lookback

#include <algorithm> /// for std::fill
#include <cmath> /// for std::isfinite
#include <cstdint> /// for int64_t, uint32_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::addressof, std::make_unique
#include <vector> /// for std::begin, std::end, std::vector

namespace tean::tests
{

template<uint32_t test_period>
void test_linear_regression_step(TeAn &fixture, decimal const testPriceStep)
{
   constexpr auto testLookbackPeriod{linear_regression<test_period>::lookback_period,};
   constexpr auto testIterationsNumber{test_period * 10,};
   auto const testPrices{std::make_unique<double[]>(testLookbackPeriod + testIterationsNumber),};
   auto const testIntercepts{std::make_unique<testing::Matcher<double>[]>(testIterationsNumber),};
   auto const testSlopes{std::make_unique<testing::Matcher<double>[]>(testIterationsNumber),};
   {
      linear_regression<test_period> testIndicator{};
      auto const testPricePrecision{inverted_power_of_ten[std::max<uint32_t>(6, testPriceStep.scale)],};
      double const testPriceStepValue{testPriceStep,};
      for (uint32_t testIteration{0,}; testIteration < testLookbackPeriod; ++testIteration)
      {
         auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(100, 1000),};
         auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
         ASSERT_FALSE(std::isfinite(testCalcValue.intercept));
         ASSERT_FALSE(std::isfinite(testCalcValue.slope));
         testPrices[testIteration] = testPrice;
      }
      auto const testValues{std::make_unique<testing::Matcher<double>[]>(testIterationsNumber),};
      auto const testForecasts{std::make_unique<testing::Matcher<double>[]>(testIterationsNumber),};
      for (uint32_t testIteration{0,}; testIteration < testIterationsNumber; ++testIteration)
      {
         auto const testPrice{testPriceStepValue * fixture.random_number<int64_t>(100, 1000),};
         auto const testCalcValue{testIndicator.calc(testLookbackPeriod + testIteration, testPrice),};
         ASSERT_TRUE(std::isfinite(testCalcValue.intercept));
         ASSERT_TRUE(std::isfinite(testCalcValue.slope));
         testPrices[testLookbackPeriod + testIteration] = testPrice;
         testIntercepts[testIteration] = testing::DoubleNear(testCalcValue.intercept, testPricePrecision);
         testSlopes[testIteration] = testing::DoubleNear(testCalcValue.slope, testPricePrecision);
         testValues[testIteration] = testing::DoubleNear(testCalcValue.slope * (test_period - 1) + testCalcValue.intercept, testPricePrecision);
         testForecasts[testIteration] = testing::DoubleNear(testCalcValue.slope * test_period + testCalcValue.intercept, testPricePrecision);
      }
      auto const testInterceptsMatcher{testing::ElementsAreArray(testIntercepts.get(), testIterationsNumber),};
      auto const testSlopesMatcher{testing::ElementsAreArray(testSlopes.get(), testIterationsNumber),};
      auto const testValuesMatcher{testing::ElementsAreArray(testValues.get(), testIterationsNumber),};
      auto const testForecastsMatcher{testing::ElementsAreArray(testForecasts.get(), testIterationsNumber),};
      std::vector<double> expectedValues;
      expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
      {
         ASSERT_EQ(TA_LINEARREG_Lookback(static_cast<int>(test_period)), static_cast<int>(testLookbackPeriod));
         int expectedFirstIndex{0,};
         int expectedNumberOfElements{0,};
         ASSERT_EQ(TA_LINEARREG(
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
         ASSERT_THAT(expectedValues, testValuesMatcher);
         testIndicator.reset();
         for (uint32_t testIteration{0,}; testIteration < testLookbackPeriod; ++testIteration)
         {
            auto const testPrice{testPrices[testIteration],};
            auto const testCalcValue{testIndicator.calc(testIteration, testPrice),};
            ASSERT_FALSE(std::isfinite(testCalcValue.intercept));
            ASSERT_FALSE(std::isfinite(testCalcValue.slope));
         }
         {
            auto const testPrice{testPrices[testLookbackPeriod],};
            auto const testCalcValue{testIndicator.calc(testLookbackPeriod, testPrice),};
            ASSERT_TRUE(std::isfinite(testCalcValue.intercept));
            ASSERT_TRUE(std::isfinite(testCalcValue.slope));
            ASSERT_THAT(expectedValues[0], testing::DoubleNear(testCalcValue.slope * (test_period - 1) + testCalcValue.intercept, testPricePrecision));
         }
      }
      std::fill(std::begin(expectedValues), std::end(expectedValues), std::numeric_limits<double>::signaling_NaN());
      {
         ASSERT_EQ(TA_LINEARREG_INTERCEPT_Lookback(static_cast<int>(test_period)), static_cast<int>(testLookbackPeriod));
         int expectedFirstIndex{0,};
         int expectedNumberOfElements{0,};
         ASSERT_EQ(TA_LINEARREG_INTERCEPT(
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
         ASSERT_THAT(expectedValues, testInterceptsMatcher);
      }
      std::fill(std::begin(expectedValues), std::end(expectedValues), std::numeric_limits<double>::signaling_NaN());
      {
         ASSERT_EQ(TA_LINEARREG_SLOPE_Lookback(static_cast<int>(test_period)), static_cast<int>(testLookbackPeriod));
         int expectedFirstIndex{0,};
         int expectedNumberOfElements{0,};
         ASSERT_EQ(TA_LINEARREG_SLOPE(
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
         ASSERT_THAT(expectedValues, testSlopesMatcher);
      }
      std::fill(std::begin(expectedValues), std::end(expectedValues), std::numeric_limits<double>::signaling_NaN());
      {
         ASSERT_EQ(TA_TSF_Lookback(static_cast<int>(test_period)), static_cast<int>(testLookbackPeriod));
         int expectedFirstIndex{0,};
         int expectedNumberOfElements{0,};
         ASSERT_EQ(TA_TSF(
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
         ASSERT_THAT(expectedValues, testForecastsMatcher);
      }
   }
   {
      linear_regression<> testIndicator{test_period,};
      ASSERT_EQ(test_period, testIndicator.period());
      ASSERT_EQ(testLookbackPeriod, testIndicator.lookback_period());
      for (uint32_t testIteration{0,}; testIteration < (testLookbackPeriod + testIterationsNumber); ++testIteration)
      {
         auto const testCalcValue{testIndicator.calc(testIteration, testPrices[testIteration]),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testCalcValue.intercept));
            ASSERT_FALSE(std::isfinite(testCalcValue.slope));
         }
         else
         {
            ASSERT_THAT(testCalcValue.intercept, testIntercepts[testIteration - testLookbackPeriod]);
            ASSERT_THAT(testCalcValue.slope, testSlopes[testIteration - testLookbackPeriod]);
         }
      }
      testIndicator.reset();
      for (uint32_t testIteration{0,}; testIteration <= testLookbackPeriod; ++testIteration)
      {
         auto const testCalcValue{testIndicator.calc(testIteration, testPrices[testIteration]),};
         if (testLookbackPeriod > testIteration)
         {
            ASSERT_FALSE(std::isfinite(testCalcValue.intercept));
            ASSERT_FALSE(std::isfinite(testCalcValue.slope));
         }
         else
         {
            ASSERT_THAT(testCalcValue.intercept, testIntercepts[testIteration - testLookbackPeriod]);
            ASSERT_THAT(testCalcValue.slope, testSlopes[testIteration - testLookbackPeriod]);
         }
      }
   }
}

template<uint32_t test_period>
void test_linear_regression(TeAn &fixture, decimal const testPriceStep)
{
   test_linear_regression_step<test_period>(fixture, testPriceStep);
   if constexpr (2 < test_period)
   {
      test_linear_regression<test_period - 1>(fixture, testPriceStep);
   }
}

TEST_F(TeAn, LinearRegression)
{
   constexpr uint32_t testMaxPeriod{100,};
   ASSERT_NO_FATAL_FAILURE(test_linear_regression<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12,}));
   ASSERT_NO_FATAL_FAILURE(test_linear_regression<testMaxPeriod>(*this, decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00,}));
}

}
