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

#include "decimal.tests.hpp" /// for tean::tests::decimal, tean::tests::inverted_power_of_ten, tean::tests::power_of_ten
#include "tean.tests.hpp" /// for tean::tests::TeAn

#include "tean/accumulation_distribution_line.hpp" /// for the test target

#include <gmock/gmock.h> /// for ASSERT_THAT, testing::DoubleNear, testing::ElementsAreArray, testing::Matcher
#include <gtest/gtest.h> /// for ASSERT_DOUBLE_EQ, ASSERT_EQ, ASSERT_FALSE, ASSERT_NO_FATAL_FAILURE
#include <indicators.h> /// for ti_ad, ti_ad_start, TI_OKAY
#include <ta_func.h> /// for TA_AD, TA_SUCCESS

#include <algorithm> /// for std::fill
#include <cmath> /// for std::isnan
#include <cstdint> /// for int64_t, uint32_t
#include <limits> /// for std::numeric_limits
#include <memory> /// for std::addressof, std::make_unique
#include <vector> /// for std::begin, std::end, std::vector

namespace tean::tests
{

TEST_F(TeAn, AccumulationDistributionLine)
{
   constexpr uint32_t testIterationsNumber = 100;
   auto const testStep = [&] (decimal const &testPriceStep, decimal const &testLotSize)
   {
      auto const testPricePrecision = inverted_power_of_ten[testPriceStep.scale / 2] * inverted_power_of_ten[testLotSize.scale / 2];
      auto const testPriceStepValue = static_cast<double>(testPriceStep);
      auto const testLotSizeValue = static_cast<double>(testLotSize);
      accumulation_distribution_line testIndicator;
      auto testHighPrices = std::make_unique<double[]>(testIterationsNumber);
      auto testLowPrices = std::make_unique<double[]>(testIterationsNumber);
      auto testClosePrices = std::make_unique<double[]>(testIterationsNumber);
      auto testTradedVolumes = std::make_unique<double[]>(testIterationsNumber);
      auto testValues = std::make_unique<testing::Matcher<double>[]>(testIterationsNumber);
      for (uint32_t testIteration = 0; testIteration < testIterationsNumber; ++testIteration)
      {
         auto const testClosePrice = testPriceStepValue * random_number<int64_t>(power_of_ten[testPriceStep.scale], power_of_ten[testPriceStep.scale + 2]);
         auto const testHighPrice = testClosePrice + testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testLowPrice = testClosePrice - testPriceStepValue * random_number<int64_t>(0, 50);
         auto const testTradedVolume = testLotSizeValue * random_number<int64_t>(10, 100);
         auto const testPickValue = testIndicator.pick(testIteration, testHighPrice, testLowPrice, testClosePrice, testTradedVolume);
         ASSERT_FALSE(std::isnan(testPickValue));
         auto const testCalcValue = testIndicator.calc(testIteration, testHighPrice, testLowPrice, testClosePrice, testTradedVolume);
         ASSERT_FALSE(std::isnan(testCalcValue));
         ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
         testHighPrices[testIteration] = testHighPrice;
         testLowPrices[testIteration] = testLowPrice;
         testClosePrices[testIteration] = testClosePrice;
         testTradedVolumes[testIteration] = testTradedVolume;
         testValues[testIteration] = testing::DoubleNear(testCalcValue, testPricePrecision);
      }
      auto const testMatcher = testing::ElementsAreArray(testValues.get(), testIterationsNumber);
      std::vector<double> expectedValues;
      expectedValues.resize(testIterationsNumber, std::numeric_limits<double>::quiet_NaN());
      {
         int expectedFirstIndex = 0;
         int expectedNumberOfElements = 0;
         ASSERT_EQ(TA_AD(
            0,
            static_cast<int>(testIterationsNumber - 1),
            testHighPrices.get(),
            testLowPrices.get(),
            testClosePrices.get(),
            testTradedVolumes.get(),
            std::addressof(expectedFirstIndex),
            std::addressof(expectedNumberOfElements),
            expectedValues.data()
         ), TA_SUCCESS);
         ASSERT_EQ(expectedFirstIndex, 0);
         ASSERT_EQ(expectedNumberOfElements, static_cast<int>(testIterationsNumber));
         ASSERT_THAT(expectedValues, testMatcher);
         testIndicator.reset();
         {
            auto const testHighPrice = testHighPrices[0];
            auto const testLowPrice = testLowPrices[0];
            auto const testClosePrice = testClosePrices[0];
            auto const testTradedVolume = testTradedVolumes[0];
            auto const testPickValue = testIndicator.pick(0, testHighPrice, testLowPrice, testClosePrice, testTradedVolume);
            ASSERT_FALSE(std::isnan(testPickValue));
            auto const testCalcValue = testIndicator.calc(0, testHighPrice, testLowPrice, testClosePrice, testTradedVolume);
            ASSERT_FALSE(std::isnan(testCalcValue));
            ASSERT_DOUBLE_EQ(testPickValue, testCalcValue);
            ASSERT_THAT(expectedValues[0], testing::DoubleNear(testCalcValue, testPricePrecision));
         }
      }
      std::fill(std::begin(expectedValues), std::end(expectedValues), std::numeric_limits<double>::quiet_NaN());
      expectedValues.resize(testIterationsNumber);
      {
         double *testInputs[] = {testHighPrices.get(), testLowPrices.get(), testClosePrices.get(), testTradedVolumes.get()};
         double *testOutputs[] = {expectedValues.data()};
         ASSERT_EQ(TI_OKAY, ti_ad(static_cast<int>(testIterationsNumber), testInputs, nullptr, testOutputs));
         ASSERT_EQ(0, ti_ad_start(nullptr));
         ASSERT_THAT(expectedValues, testMatcher);
      }
   };
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 12}, decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 0}));
   ASSERT_NO_FATAL_FAILURE(testStep(decimal{.value = static_cast<int64_t>(power_of_ten[6]), .scale = 00}, decimal{.value = static_cast<int64_t>(power_of_ten[0]), .scale = 6}));
}

}
