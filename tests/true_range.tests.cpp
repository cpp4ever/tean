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

#include "decimal.tests.hpp" /// for tean::tests::decimal, tean::tests::power_of_ten
#include "tean.tests.hpp" /// for tean::tests::TeAn

#include "tean/true_range.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleEq, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_DOUBLE_EQ, ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE
#include <indicators.h> /// for TI_OKAY, ti_tr, ti_tr_start
#include <ta_func.h> /// for TA_SUCCESS, TA_TRANGE

#include <algorithm> /// for std::fill
#include <cmath> /// for std::isnan
#include <cstdint> /// for int64_t, uint32_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::addressof, std::make_unique
#include <vector> /// for std::begin, std::end, std::vector

namespace tean::tests
{

TEST_F(TeAn, TrueRange)
{
   constexpr uint32_t testIterationsNumber = 100;
   auto const testStep = [&] (decimal const &testPriceStep)
   {
      auto const testPriceStepValue = static_cast<double>(testPriceStep);
      true_range testIndicator;
      auto testHighPrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
      auto testLowPrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
      auto testClosePrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
      for (uint32_t testIteration = 0; testIteration < testIndicator.lookback_period(); ++testIteration)
      {
         auto const testClosePrice = testPriceStepValue * random_number<int64_t>(100, 1000);
         auto const testHighPrice = testClosePrice + testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testLowPrice = testClosePrice - testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testPickValue = testIndicator.pick(testIteration, testHighPrice, testLowPrice, testClosePrice);
         ASSERT_TRUE(std::isnan(testPickValue));
         auto const testCalcValue = testIndicator.calc(testIteration, testHighPrice, testLowPrice, testClosePrice);
         ASSERT_TRUE(std::isnan(testCalcValue));
         testHighPrices[testIteration] = testHighPrice;
         testLowPrices[testIteration] = testLowPrice;
         testClosePrices[testIteration] = testClosePrice;
      }
      auto testValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
      for (uint32_t testIteration = 0; testIteration < testIterationsNumber; ++testIteration)
      {
         auto const testClosePrice = testPriceStepValue * random_number<int64_t>(100, 1000);
         auto const testHighPrice = testClosePrice + testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testLowPrice = testClosePrice - testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testPickValue = testIndicator.pick(testIndicator.lookback_period() + testIteration, testHighPrice, testLowPrice, testClosePrice);
         ASSERT_FALSE(std::isnan(testPickValue));
         auto const testCalcValue = testIndicator.calc(testIndicator.lookback_period() + testIteration, testHighPrice, testLowPrice, testClosePrice);
         ASSERT_FALSE(std::isnan(testCalcValue));
         ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
         testHighPrices[testIndicator.lookback_period() + testIteration] = testHighPrice;
         testLowPrices[testIndicator.lookback_period() + testIteration] = testLowPrice;
         testClosePrices[testIndicator.lookback_period() + testIteration] = testClosePrice;
         testValues[testIteration] = testing::DoubleEq(testCalcValue);
      }
      auto const testMatcher = testing::ElementsAreArray(testValues.get(), testIterationsNumber);
      std::vector<double> expectedValues;
      expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
      {
         int expectedFirstIndex = 0;
         int expectedNumberOfElements = 0;
         ASSERT_EQ(TA_TRANGE(
            0,
            static_cast<int>(testIndicator.lookback_period() + testIterationsNumber - 1),
            testHighPrices.get(),
            testLowPrices.get(),
            testClosePrices.get(),
            std::addressof(expectedFirstIndex),
            std::addressof(expectedNumberOfElements),
            expectedValues.data()
         ), TA_SUCCESS);
         ASSERT_EQ(expectedFirstIndex, 1);
         ASSERT_EQ(expectedNumberOfElements, static_cast<int>(testIterationsNumber));
         ASSERT_THAT(expectedValues, testMatcher);
         testIndicator.reset();
         for (uint32_t testIteration = 0; testIteration < testIndicator.lookback_period(); ++testIteration)
         {
            auto const testHighPrice = testHighPrices[testIteration];
            auto const testLowPrice = testLowPrices[testIteration];
            auto const testClosePrice = testClosePrices[testIteration];
            auto const testPickValue = testIndicator.pick(testIteration, testHighPrice, testLowPrice, testClosePrice);
            ASSERT_TRUE(std::isnan(testPickValue));
            auto const testCalcValue = testIndicator.calc(testIteration, testHighPrice, testLowPrice, testClosePrice);
            ASSERT_TRUE(std::isnan(testCalcValue));
         }
         {
            auto const testHighPrice = testHighPrices[testIndicator.lookback_period()];
            auto const testLowPrice = testLowPrices[testIndicator.lookback_period()];
            auto const testClosePrice = testClosePrices[testIndicator.lookback_period()];
            auto const testPickValue = testIndicator.pick(testIndicator.lookback_period(), testHighPrice, testLowPrice, testClosePrice);
            ASSERT_FALSE(std::isnan(testPickValue));
            auto const testCalcValue = testIndicator.calc(testIndicator.lookback_period(), testHighPrice, testLowPrice, testClosePrice);
            ASSERT_FALSE(std::isnan(testCalcValue));
            ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            ASSERT_THAT(expectedValues[0], testing::DoubleEq(testCalcValue));
         }
      }
      std::fill(std::begin(expectedValues), std::end(expectedValues), std::numeric_limits<double>::signaling_NaN());
      expectedValues.resize(testIterationsNumber + 1);
      {
         double *testInputs[] = {testHighPrices.get(), testLowPrices.get(), testClosePrices.get()};
         double *testOutputs[] = {expectedValues.data()};
         ASSERT_EQ(TI_OKAY, ti_tr(static_cast<int>(testIndicator.lookback_period() + testIterationsNumber), testInputs, nullptr, testOutputs));
         ASSERT_EQ(0, ti_tr_start(nullptr));
         expectedValues.erase(std::begin(expectedValues));
         ASSERT_THAT(expectedValues, testMatcher);
      }
   };
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12}));
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00}));
}

}
