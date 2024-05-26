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

#include "tean/ease_of_movement.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleEq, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_DOUBLE_EQ, ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE, ASSERT_TRUE
#include <indicators.h> /// for ti_emv, ti_emv_start, TI_OKAY

#include <cmath> /// for std::isnan
#include <cstdint> /// for int64_t, uint32_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::make_unique
#include <vector> /// for std::vector

namespace tean::tests
{

TEST_F(TeAn, EaseOfMovement)
{
   constexpr uint32_t testIterationsNumber = 100;
   auto const testStep = [&] (decimal const &testPriceStep, decimal const &testLotSize)
   {
      auto const testPriceStepValue = static_cast<double>(testPriceStep);
      auto const testLotSizeValue = static_cast<double>(testLotSize);
      ease_of_movement testIndicator;
      auto testHighPrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
      auto testLowPrices = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
      auto testTradedVolumes = std::make_unique<double[]>(testIndicator.lookback_period() + testIterationsNumber);
      ASSERT_EQ(ti_emv_start(nullptr), static_cast<int>(testIndicator.lookback_period()));
      for (uint32_t testIteration = 0; testIteration < testIndicator.lookback_period(); ++testIteration)
      {
         auto const testClosePrice = testPriceStepValue * random_number<int64_t>(100, 1000);
         auto const testHighPrice = testClosePrice + testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testLowPrice = testClosePrice - testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testTradedVolume = testLotSizeValue * random_number<int64_t>(10, 100);
         auto const testPickValue = testIndicator.pick(testIteration, testHighPrice, testLowPrice, testTradedVolume);
         ASSERT_TRUE(std::isnan(testPickValue));
         auto const testCalcValue = testIndicator.calc(testIteration, testHighPrice, testLowPrice, testTradedVolume);
         ASSERT_TRUE(std::isnan(testCalcValue));
         testHighPrices[testIteration] = testHighPrice;
         testLowPrices[testIteration] = testLowPrice;
         testTradedVolumes[testIteration] = testTradedVolume;
      }
      auto testValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
      for (uint32_t testIteration = 0; testIteration < testIterationsNumber; ++testIteration)
      {
         auto const testClosePrice = testPriceStepValue * random_number<int64_t>(100, 1000);
         auto const testHighPrice = testClosePrice + testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testLowPrice = testClosePrice - testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testTradedVolume = testLotSizeValue * random_number<int64_t>(10, 100);
         auto const testPickValue = testIndicator.pick(testIndicator.lookback_period() + testIteration, testHighPrice, testLowPrice, testTradedVolume);
         ASSERT_FALSE(std::isnan(testPickValue));
         auto const testCalcValue = testIndicator.calc(testIndicator.lookback_period() + testIteration, testHighPrice, testLowPrice, testTradedVolume);
         ASSERT_FALSE(std::isnan(testCalcValue));
         ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
         testHighPrices[testIndicator.lookback_period() + testIteration] = testHighPrice;
         testLowPrices[testIndicator.lookback_period() + testIteration] = testLowPrice;
         testTradedVolumes[testIndicator.lookback_period() + testIteration] = testTradedVolume;
         testValues[testIteration] = testing::DoubleEq(testCalcValue);
      }
      auto const testMatcher = testing::ElementsAreArray(testValues.get(), testIterationsNumber);
      std::vector<double> expectedValues;
      expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::signaling_NaN());
      {
         double *testInputs[] = {testHighPrices.get(), testLowPrices.get(), testTradedVolumes.get()};
         double *testOutputs[] = {expectedValues.data()};
         ASSERT_EQ(TI_OKAY, ti_emv(static_cast<int>(testIndicator.lookback_period() + testIterationsNumber), testInputs, nullptr, testOutputs));
         ASSERT_EQ(1, ti_emv_start(nullptr));
         ASSERT_THAT(expectedValues, testMatcher);
         testIndicator.reset();
         for (uint32_t testIteration = 0; testIteration < testIndicator.lookback_period(); ++testIteration)
         {
            auto const testHighPrice = testHighPrices[testIteration];
            auto const testLowPrice = testLowPrices[testIteration];
            auto const testTradedVolume = testTradedVolumes[testIteration];
            auto const testPickValue = testIndicator.pick(testIteration, testHighPrice, testLowPrice, testTradedVolume);
            ASSERT_TRUE(std::isnan(testPickValue));
            auto const testCalcValue = testIndicator.calc(testIteration, testHighPrice, testLowPrice, testTradedVolume);
            ASSERT_TRUE(std::isnan(testCalcValue));
         }
         {
            auto const testHighPrice = testHighPrices[testIndicator.lookback_period()];
            auto const testLowPrice = testLowPrices[testIndicator.lookback_period()];
            auto const testTradedVolume = testTradedVolumes[testIndicator.lookback_period()];
            auto const testPickValue = testIndicator.pick(testIndicator.lookback_period(), testHighPrice, testLowPrice, testTradedVolume);
            ASSERT_FALSE(std::isnan(testPickValue));
            auto const testCalcValue = testIndicator.calc(testIndicator.lookback_period(), testHighPrice, testLowPrice, testTradedVolume);
            ASSERT_FALSE(std::isnan(testCalcValue));
            ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            ASSERT_DOUBLE_EQ(expectedValues[0], testCalcValue);
         }
      }
   };
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12}, decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 0}));
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00}, decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 6}));
}

}
